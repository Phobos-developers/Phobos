
#include <ScenarioClass.h>
#include <ThemeClass.h>
#include <WWMouseClass.h>
#include <Drawing.h>
#include <BitFont.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Scenario/Body.h>
#include <ToggleClass.h>
#include <ShapeButtonClass.h>

// SHP & PCX drawing support
void DrawImage(
	DSurface* pSurface,
	RectangleStruct destinationRect,
	BSurface* pPCXSurface,
	SHPStruct* fileSHP,
	ConvertClass* pPalette,
	int frameIndex = 0,
	int zAdjust = 0,
	BlitterFlags blitterFlags = BlitterFlags::None)
{
	if (!pSurface || (!pPCXSurface && !fileSHP))
		return;

	bool painted = false;

	// Prioritize drawing the PCX file if it's provided
	if (pPCXSurface)
	{
		// This function handles stretching the PCX to fit the destinationRect
		PCX::Instance.BlitToSurface(&destinationRect, pSurface, pPCXSurface);
		painted = true;
	}
	// Otherwise, if an SHP is provided, draw it
	else if (fileSHP)
	{
		// SHP drawing requires a palette converter
		if (!pPalette)
		{
			Debug::Log("DrawImage Error: Attempted to draw SHP without providing a pPalette.\n");
			return;
		}

		Point2D sourcePosition = { 0, 0 };

		CC_Draw_Shape(
			pSurface,
			pPalette,
			fileSHP,
			frameIndex,
			&sourcePosition,
			&destinationRect,
			BlitterFlags::None,
			0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0
		);
		painted = true;
	}

	// Use the Phobos PCX instance to blit the image
	if (painted && blitterFlags == (BlitterFlags::Darken | BlitterFlags::bf_400))
	{
		auto black = ColorStruct { 0, 0, 0 };
		int opacity = 40;
		pSurface->FillRectTrans(&destinationRect, &black, opacity);
	}

	// Other new BlitterFlags cases should be placed here so both SHP & PCS will be affected
}

DEFINE_HOOK(0x4B6C30, Dropship_Loadout_Remake, 0x0) //0x5)
{
	enum { EndFunction = 0x4B9690 };

	// Get the number of dropship's for this mission at the very beginning
	int nStartingDropships = ScenarioClass::Instance->StartingDropships;

	// If there are no dropships, there is no loadout screen. Exit immediately
	if (nStartingDropships == 0 || nStartingDropships > 3)
		return EndFunction;

	// Clear the off-screen buffer to black now that we know the screen will be displayed
	DSurface::Hidden->Fill(0);

	// --- FILENAME INITIALIZATION ---

	char tempFilenameBuffer[32];
	_snprintf_s(tempFilenameBuffer, sizeof(tempFilenameBuffer), "DROP%04d.SHP", nStartingDropships);
	char* file_Background = _strdup(tempFilenameBuffer);

	// Declare the filenames for all other graphical assets.
	char* file_Loadout = _strdup("LOADOUT.SHP");
	char* file_PilotLit = _strdup("PILOTLIT.SHP");
	char* file_NotAvailableIcon = _strdup("XXICON.SHP");
	char* file_UpArrow = _strdup("DROPUP.SHP");
	char* file_DownArrow = _strdup("DROPDOWN.SHP");

	char* file_DropshipPalette = _strdup("DROPSHIP.PAL");
	char* file_CameoPalette = _strdup("CAMEO.PAL");

	//char* file_Pilot_1 = _strdup("DGREEN1.SHP");
	//char* file_Pilot_2 = _strdup("DGREEN2.SHP");
	//char* file_Pilot_3 = _strdup("DGREEN3.SHP");
	//char* file_Pilot_4 = _strdup("DGREEN4.SHP");

	// Group the pilot portrait filenames into an array for easy iteration.
	//char* pilot_portraits[] = {
	//	file_Pilot_Green, file_Pilot_Red, file_Pilot_Blue, file_Pilot_Yellow
	//};
	
	// --- PRE-LOOP SETUP: MUSIC, MOUSE, AND MONEY ---

	// Initial EVA Voice
	VoxClass::PlayIndex(ScenarioExt::Global()->DropshipLoadoutStartEVA.Get(-1));

	// Play the specific theme for the dropship loadout screen
	const int theme = ScenarioExt::Global()->DropshipLoadoutTheme;
	if (theme == -1)
		ThemeClass::Instance.Stop(true);
	else
		ThemeClass::Instance.Play(theme);

	// Reactivate the mouse cursor for the new UI screen
	WWMouseClass::Instance->HideCursor();
	WWMouseClass::Instance->ShowCursor();
	WWMouseClass::Instance->CaptureMouse();
	WWMouseClass::Instance->RefCount = 0;

	// Get initial money for the loadout
	long dropshipLoadoutMoney = ScenarioExt::Global()->DropshipLoadoutMoney >= 0 ? ScenarioExt::Global()->DropshipLoadoutMoney : HouseClass::CurrentPlayer->Available_Money();

	// --- BUILD AVAILABLE UNIT LIST ---
	// This logic now directly adds all units from the scenario's [AllowableUnits] list,
	// or falls back to all standard units if the list is empty

	std::vector<TechnoTypeClass*> availableUnits;
	std::vector<int> allowableUnitMaximums;

	if (ScenarioClass::Instance->AllowableUnits.Count > 0)
	{
		if (ScenarioClass::Instance->AllowableUnitMaximums.Count > 0
			&& ScenarioClass::Instance->AllowableUnits.Count != ScenarioClass::Instance->AllowableUnitMaximums.Count)
		{
			Debug::Log("Dropship Loadout - AllowableUnits and AllowableUnitMaximums must have the same number of elements. Units list disabled.\n");
		}
		else
		{
			for (int i = 0; i < ScenarioClass::Instance->AllowableUnits.Count; ++i)
			{
				if (ScenarioClass::Instance->AllowableUnitMaximums.Items[i] != 0)
				{
					int maximumCount = ScenarioClass::Instance->AllowableUnitMaximums.Items[i];

					if (maximumCount == 0)
						continue;

					allowableUnitMaximums.push_back(maximumCount);
				}
				TechnoTypeClass* pType = ScenarioClass::Instance->AllowableUnits.Items[i];
				availableUnits.push_back(pType);
			}
		}
	}
	else
	{
		// If [AllowableUnits] is empty, fall back to all standard units.
		for (const auto pType : TechnoTypeClass::Array)
		{
			if (pType->WhatAmI() == AbstractType::InfantryType || (pType->WhatAmI() == AbstractType::UnitType))
				availableUnits.push_back(pType);
		}
	}

	// --- LOAD & PREPARE MAIN UI ASSETS ---
	// Load the core graphical assets and calculate their on-screen positions.

	const int cameoWidth = 60, cameoHeight = 48;

	SHPStruct* screenBG_Image = FileSystem::LoadSHPFile(file_Background);
	if (!screenBG_Image)
		Debug::Log("Dropship Loadout - Missing background '%s'.\n", file_Background);

	ConvertClass* screenBG_pal = FileSystem::LoadPALFile(file_DropshipPalette, DSurface::Hidden);

	SHPStruct* notAvailableIcon_Image = FileSystem::LoadSHPFile(file_NotAvailableIcon);

	if (!notAvailableIcon_Image)
		Debug::Log("Dropship Loadout - Missing NO CAMEO image '%s'.\n", file_NotAvailableIcon);

	ConvertClass* notAvailableIcon_pal = FileSystem::LoadPALFile(file_CameoPalette, DSurface::Hidden);

	SHPStruct* upArrow_Image = FileSystem::LoadSHPFile(file_UpArrow);

	if (!upArrow_Image)
		Debug::Log("Dropship Loadout - Missing UP ARROW image '%s'.\n", file_UpArrow);

	SHPStruct* downArrow_Image = FileSystem::LoadSHPFile(file_DownArrow);

	if (!downArrow_Image)
		Debug::Log("Dropship Loadout - Missing DOWN ARROW image '%s'.\n", file_DownArrow);

	// Basic location data

	int backgroundWidth = screenBG_Image->Width;
	int backgroundHeight = screenBG_Image->Height;

	// Calculate the top-left corner coordinates to center the image.
	int backgroundX = (DSurface::Hidden->GetWidth() - backgroundWidth) / 2;
	int backgroundY = (DSurface::Hidden->GetHeight() - backgroundHeight) / 2;
	int screenWidth = backgroundX + backgroundWidth;
	int screenHeight = backgroundY + backgroundHeight;

	// Store the final position and dimensions for later drawing operations.
	RectangleStruct windowRectangle = { backgroundX, backgroundY, backgroundWidth, backgroundHeight };

	// --- 5. CALCULATE UI ELEMENT POSITIONS ---
	// Pre-calculate the screen positions for dynamic UI elements like cameos.

	// Calculate positions for the sidebar cameos
	int nSidebarCameos = 8;
	std::vector<RectangleStruct> sidebarCameoLocations;

	for (int i = 0; i < nSidebarCameos; ++i)
	{
		int cameoX = backgroundX + 2 + 491 + 68 * (i % 2);
		int cameoY = backgroundY + 25 + 50 * (i / 2);
		RectangleStruct cameoRectangle = { cameoX, cameoY, cameoWidth, cameoHeight };
		sidebarCameoLocations.push_back(cameoRectangle);
	}

	// Calculate positions for the scroll arrow buttons.
	int upArrowWidth = upArrow_Image->Width;
	int upArrowHeight = upArrow_Image->Height;
	int downArrowWidth = downArrow_Image->Width;
	int downArrowHeight = downArrow_Image->Height;

	// Center point between the two cameo columns.
	int centerOfCameoColumns = sidebarCameoLocations[0].X + sidebarCameoLocations[0].Width + (sidebarCameoLocations[1].X - (sidebarCameoLocations[0].X + sidebarCameoLocations[0].Width)) / 2;

	// Y position below the last row of cameos.
	int arrowsY = sidebarCameoLocations.back().Y + sidebarCameoLocations.back().Height + 6;

	RectangleStruct upArrowLocation = {
		centerOfCameoColumns - upArrow_Image->Width, // Position left of center
		arrowsY,
		upArrow_Image->Width,
		upArrow_Image->Height
	};

	RectangleStruct downArrowLocation = {
		centerOfCameoColumns, // Position right of center
		arrowsY,
		downArrow_Image->Width,
		downArrow_Image->Height
	};

	// Calculate positions for the dropship slots
	int nDropshipBayCameos = 5;
	int nDropshipBayTotalSlots = nStartingDropships * nDropshipBayCameos;
	std::vector<std::vector<RectangleStruct>> dropshipBayCameoLocations;

	// Case 1: Slots coordinates of the Dropship #1
	if (nStartingDropships == 1 || nStartingDropships == 2)
	{
		int cameoX = backgroundX + 2 + 53;
		int cameoY = backgroundY + 69;
		std::vector<RectangleStruct> dropshipBayCameoLocationsList;

		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66 + 66, cameoY + 50, cameoWidth, cameoHeight });

		dropshipBayCameoLocations.push_back(dropshipBayCameoLocationsList);
		dropshipBayCameoLocationsList.clear();
	}

	// Case 2: Slots coordinates of the Dropship #2
	if (nStartingDropships == 2)
	{
		int cameoX = backgroundX + 2 + 53;
		int cameoY = backgroundY + 69 + 140;
		std::vector<RectangleStruct> dropshipBayCameoLocationsList;

		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66 + 66, cameoY + 50, cameoWidth, cameoHeight });

		dropshipBayCameoLocations.push_back(dropshipBayCameoLocationsList);
		dropshipBayCameoLocationsList.clear();
	}

	// Case 3: Slots coordinates of the Dropship #3
	if (nStartingDropships == 3)
	{
		int cameoX = backgroundX + 2 + 53;
		int cameoY = backgroundY + 39;
		std::vector<RectangleStruct> dropshipBayCameoLocationsList;

		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66 + 66, cameoY + 50, cameoWidth, cameoHeight });

		dropshipBayCameoLocations.push_back(dropshipBayCameoLocationsList);
		dropshipBayCameoLocationsList.clear();

		cameoY += 120;
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66 + 66, cameoY + 50, cameoWidth, cameoHeight });

		dropshipBayCameoLocations.push_back(dropshipBayCameoLocationsList);
		dropshipBayCameoLocationsList.clear();

		cameoY += 120;
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66 + 66, cameoY + 50, cameoWidth, cameoHeight });

		dropshipBayCameoLocations.push_back(dropshipBayCameoLocationsList);
		dropshipBayCameoLocationsList.clear();
	}

	// Units loaded into the dropships
	std::vector<std::vector<TechnoTypeClass*>> chosenUnits;

	// --- 6. CREATE INTERACTIVE UI BUTTONS ---
	std::vector<ShapeButtonClass*> buttonsList;

	// Create the UP arrow button first. It will act as the head of the list.
	int btn_ScrollUp_ID = 100;
	ShapeButtonClass* btn_ScrollUp = GameCreate<ShapeButtonClass>(
		btn_ScrollUp_ID,
		0, 0,
		upArrowLocation.Width, upArrowLocation.Height,
		true
	);

	btn_ScrollUp->SetPosition(upArrowLocation.X, upArrowLocation.Y);
	btn_ScrollUp->SetDimension(upArrowLocation.Width, upArrowLocation.Height);
	btn_ScrollUp->DrawPosition.X = upArrowLocation.X;
	btn_ScrollUp->DrawPosition.Y = upArrowLocation.Y;

	buttonsList.push_back(btn_ScrollUp);

	// The commandManager is a ToggleClass pointer to the first button.
	ToggleClass* commandManager = btn_ScrollUp;

	// Create the DOWN arrow button and add it to the manager's list.
	int btn_ScrollDown_ID = 101;
	ShapeButtonClass* btn_ScrollDown = GameCreate<ShapeButtonClass>(
		btn_ScrollDown_ID,
		0, 0,
		downArrowLocation.Width, downArrowLocation.Height,
		true
	);

	btn_ScrollDown->SetPosition(downArrowLocation.X, downArrowLocation.Y);
	btn_ScrollDown->SetDimension(downArrowLocation.Width, downArrowLocation.Height);
	btn_ScrollDown->DrawPosition.X = downArrowLocation.X;
	btn_ScrollDown->DrawPosition.Y = downArrowLocation.Y;
	buttonsList.push_back(btn_ScrollDown);
	commandManager->Add(*btn_ScrollDown);

	// Create the DROPSHIP CAMEOS
	int btn_BasicDropshipCameo_ID = 200;
	int newID = btn_BasicDropshipCameo_ID;
	std::vector<std::vector<TechnoTypeClass*>> dropshipBayChosenUnitsLists;
	std::map<TechnoTypeClass*, int> dropshipBayChosenUnitsCount;

	for (int i = 0; i < nStartingDropships; i++)
	{
		dropshipBayChosenUnitsLists.push_back(std::vector<TechnoTypeClass*>());

		for (int j = 0; j < nDropshipBayCameos; j++)
		{
			ShapeButtonClass* newButton = GameCreate<ShapeButtonClass>(
				newID,
				0, 0,
				cameoWidth, cameoHeight,
				true
			);

			newButton->SetPosition(dropshipBayCameoLocations[i][j].X, dropshipBayCameoLocations[i][j].Y);
			newButton->SetDimension(cameoWidth, cameoHeight);
			newButton->DrawPosition.X = dropshipBayCameoLocations[i][j].X;
			newButton->DrawPosition.Y = dropshipBayCameoLocations[i][j].Y;
			buttonsList.push_back(newButton);
			commandManager->Add(*newButton);
			dropshipBayChosenUnitsLists[i].push_back(nullptr);
			newID++;
		}
	}

	// Create the SIDEBAR CAMEOS

	int btn_BasicSidebarCameo_ID = 300;

	for (int i = 0; i < nSidebarCameos; i++)
	{
		int newID = btn_BasicSidebarCameo_ID + i;

		ShapeButtonClass* newButton = GameCreate<ShapeButtonClass>(
			newID,
			0, 0,
			cameoWidth, cameoHeight,
			true
		);

		newButton->SetPosition(sidebarCameoLocations[i].X, sidebarCameoLocations[i].Y);
		newButton->SetDimension(cameoWidth, cameoHeight);
		newButton->DrawPosition.X = sidebarCameoLocations[i].X;
		newButton->DrawPosition.Y = sidebarCameoLocations[i].Y;
		buttonsList.push_back(newButton);
		commandManager->Add(*newButton);
	}

	Point2D noLocation = { 0, 0 };
	TechnoTypeClass* lastSelected = nullptr;

	// --- 7. MAIN INTERACTIVE LOOP ---
	bool done = false;
	bool repaintAll = true; // Force initial draw
	int firstBrowsableCameo = 0; // Points to the first element in the sidebar to be drawed. Arrows modify this index

	commandManager->TurnOn();
	int countdown = 0; // TO-DO: Check if I can remove it now
	int keyDelayTimer = 5; // TO-DO: Check if I can remove it now
	bool lastTimeWasOverCameos = false;
	int totalDropshipSlots = nStartingDropships * nDropshipBayCameos;

	while (!done)
	{
		Game::CallBack();

		if (countdown > 0)
			countdown--;

		// Get input
		int command = commandManager->Input();
		if (command != 0)// && command != 1 && command != 2 && command != 2048 && command != 2049 && command != 2050 && command != 2086 && command != 2088)
			Debug::Log("Command: %d\n", command);
		//Debug::Log("%d\n", countdown);
		int buttonID = -1;
		// Check if a mouse click has happened inside a button.
		// If so, it overrides any keyboard command from this frame. 
		//if (command > 0) // Mouse click with the left button
		{
			RectangleStruct mouseRect = WWMouseClass::Instance->Rect2;
			//Point2D mousePos = { 0, 0 };
			//WWMouseClass::Instance->GetCoords(&mousePos);

			for (auto button : buttonsList)
			{
				if (mouseRect.X >= button->X
					&& mouseRect.X <= (button->X + button->Width)
					&& mouseRect.Y >= button->Y
					&& mouseRect.Y <= (button->Y + button->Height))
				{
					buttonID = button->ID;
					break;
				}
			}
		}

		// Key/Mouse click input values
		bool pressedLeftClick = command == 1;
		bool pressedRightClick = command == 2;

		bool isAnySidebarCameo = buttonID >= btn_BasicSidebarCameo_ID && buttonID < (btn_BasicSidebarCameo_ID + nSidebarCameos);
		bool isHoveringOverSidebarCameos = command == 0 && isAnySidebarCameo;
		bool pressedAnySidebarCameo = pressedLeftClick && isAnySidebarCameo;
		bool pressedAnySidebarCameoWithRigthClick = pressedRightClick && isAnySidebarCameo;
		int mouseOverSidebarCameoID = isHoveringOverSidebarCameos ? buttonID : -1;

		bool isAnyDropshipCameo = buttonID >= btn_BasicDropshipCameo_ID && buttonID < (btn_BasicDropshipCameo_ID + nDropshipBayTotalSlots);
		bool isHoveringOverDropshipCameos = command == 0 && isAnyDropshipCameo;
		bool pressedAnyDropshipCameo = pressedLeftClick && isAnyDropshipCameo;
		int mouseOverDropshipCameoID = isHoveringOverDropshipCameos ? buttonID : -1;

		bool isUpArrow = buttonID == btn_ScrollUp_ID;
		bool isDownArrow = buttonID == btn_ScrollDown_ID;
		bool pressedUpArrow = command == VK_UP || ((pressedLeftClick || command ==(32768 + btn_ScrollUp_ID)) && isUpArrow);
		bool pressedDownArrow = command == VK_DOWN || (pressedLeftClick && isDownArrow);
		bool pressedAnyArrow = pressedUpArrow || pressedDownArrow;

		if (pressedUpArrow)
			command = btn_ScrollUp_ID;
		else if (pressedDownArrow)
			command = btn_ScrollDown_ID;
		else if (pressedAnySidebarCameo || pressedAnyDropshipCameo || pressedAnySidebarCameoWithRigthClick)
			command = buttonID;

		bool validSidebarCameoPurchase = false;
		bool freeDropshipSlots = false;
		Point2D mouseLocationInDropshipCameos = { 0, 0 };

		// Finding where is the first free slot in the Dropships & insert the unit if the free slot is found
		for (int i = 0; i < dropshipBayCameoLocations.size() && !freeDropshipSlots; i++)
		{
			for (int j = 0; j < dropshipBayCameoLocations[i].size() && !freeDropshipSlots; j++)
			{
				if (dropshipBayChosenUnitsLists[i][j])
					continue;

				// Finally found a free slot without units
				freeDropshipSlots = true;
				break;
			}
		}

		if (isHoveringOverSidebarCameos || pressedAnySidebarCameo)
		{
			int sidebarIndex = firstBrowsableCameo + (buttonID - btn_BasicSidebarCameo_ID);

			if (sidebarIndex < availableUnits.size())
			{
				auto const pType = availableUnits[sidebarIndex];
				int maxInstances = allowableUnitMaximums[sidebarIndex] < 0 ? INT_MAX : allowableUnitMaximums[sidebarIndex];
				int nInstances = dropshipBayChosenUnitsCount.count(pType) > 0 ? dropshipBayChosenUnitsCount[pType] : 0;

				if (nInstances < maxInstances
					&& pType->Cost <= dropshipLoadoutMoney
					&& freeDropshipSlots)
				{
					validSidebarCameoPurchase = true;
				}
			}
		}

		// Locating the dropship cameo with the mouse cursor
		if (isHoveringOverDropshipCameos)
		{
			bool found = false;

			for (int i = 0; i < dropshipBayCameoLocations.size() && !found; i++)
			{
				for (int j = 0; j < dropshipBayCameoLocations[i].size() && !found; j++)
				{
					int dropshipIndex = (mouseOverDropshipCameoID - btn_BasicDropshipCameo_ID) / nDropshipBayCameos;
					int slotIndex = mouseOverDropshipCameoID - btn_BasicDropshipCameo_ID - (dropshipIndex * nDropshipBayCameos);

					if (i == dropshipIndex && j == slotIndex)
					{
						mouseLocationInDropshipCameos = { i, j };
						found = true;
						break;
					}
				}
			}
		}

		// Check if pressed UP or DOWN keys and update the index of the first element to be showed in the sidebar
		if (pressedUpArrow) // UP key or click in the UP button
		{
			if (firstBrowsableCameo >= 2)
			{
				firstBrowsableCameo -= 2;
				countdown = keyDelayTimer;
				repaintAll = true;

				// Click sound
				VocClass::PlayGlobal(RulesClass::Instance->GUITabSound, 0x2000, 1.0);
			}
		}
		else if (pressedDownArrow) // DOWN key or click in the DOWN button
		{
			if (availableUnits.size() > (firstBrowsableCameo + nSidebarCameos))
			{
				firstBrowsableCameo += 2;
				countdown = keyDelayTimer;
				repaintAll = true;

				// Click sound
				VocClass::PlayGlobal(RulesClass::Instance->GUITabSound, 0x2000, 1.0);
			}
		}
		else if (pressedAnySidebarCameoWithRigthClick)
		{
			// Remove the last dropship unit instance of the right-clicked unit in the sidebar
			int newIndex = firstBrowsableCameo + (command - btn_BasicSidebarCameo_ID);
			auto const pType = availableUnits[newIndex];
			bool found = false;

			for (int i = dropshipBayChosenUnitsLists.size() - 1; i >= 0 && !found; --i)
			{
				auto& dropshipBay = dropshipBayChosenUnitsLists[i];

				for (int j = dropshipBay.size() - 1; j >= 0 && !found; --j)
				{
					if (dropshipBay[j] == pType)
					{
						dropshipLoadoutMoney += pType->Cost;

						dropshipBay.erase(dropshipBay.begin() + j);
						dropshipBay.push_back(nullptr);
						found = true;
						repaintAll = true;

						// Update unit's count in the dropships
						if (dropshipBayChosenUnitsCount.count(pType) > 0)
							--dropshipBayChosenUnitsCount[pType];
						else
							dropshipBayChosenUnitsCount[pType] = 0;

						// Click sound
						VocClass::PlayGlobal(RulesClass::Instance->SellSound, 0x2000, 1.0);

						break;
					}
				}
			}

		}
		else if (pressedAnySidebarCameo)
		{
			// Findig what unit must be loaded in the Dropships free slots
			int newIndex = firstBrowsableCameo + (command - btn_BasicSidebarCameo_ID);

			if (validSidebarCameoPurchase)
			{
				auto const pType = availableUnits[newIndex];
				bool foundFreeSlot = false;

				// Finding where is the first free slot in the Dropships & insert the unit if the free slot is found
				for (int i = 0; i < dropshipBayCameoLocations.size() && !foundFreeSlot; i++)
				{
					for (int j = 0; j < dropshipBayCameoLocations[i].size() && !foundFreeSlot; j++)
					{
						auto const pDropshipSlotType = dropshipBayChosenUnitsLists[i][j];

						if (pDropshipSlotType)
							continue;

						dropshipBayChosenUnitsLists[i][j] = pType;
						dropshipLoadoutMoney -= pType->Cost;
						foundFreeSlot = true;
						lastSelected = pType;

						// Update unit's count in the dropships
						++dropshipBayChosenUnitsCount[pType];

						// Click sound
						VocClass::PlayGlobal(RulesClass::Instance->GenericClick, 0x2000, 1.0);
						break;
					}
				}

				if (foundFreeSlot)
					repaintAll = true;
			}

			//validSidebarCameoPurchase
			/*if (newIndex < availableUnits.size())
			{
				auto const pType = availableUnits[newIndex];
				int maxInstances = allowableUnitMaximums[newIndex] < 0 ? INT_MAX : allowableUnitMaximums[newIndex];
				int nInstances = dropshipBayChosenUnitsCount.count(pType) > 0 ? dropshipBayChosenUnitsCount[pType] : 0;

				if (nInstances < maxInstances && pType->Cost <= dropshipLoadoutMoney)
				{
					bool foundFreeSlot = false;

					// Finding where is the first free slot in the Dropships & insert the unit if the free slot is found
					for (int i = 0; i < dropshipBayCameoLocations.size() && !foundFreeSlot; i++)
					{
						for (int j = 0; j < dropshipBayCameoLocations[i].size() && !foundFreeSlot; j++)
						{
							auto const pDropshipSlotType = dropshipBayChosenUnitsLists[i][j];

							if (pDropshipSlotType)
								continue;

							dropshipBayChosenUnitsLists[i][j] = pType;
							dropshipLoadoutMoney -= pType->Cost;
							foundFreeSlot = true;
							lastSelected = pType;

							// Update unit's count in the dropships
							++dropshipBayChosenUnitsCount[pType];
							break;
						}
					}

					if (foundFreeSlot)
						repaintAll = true;
				}
			}*/
		}
		else if (pressedAnyDropshipCameo)
		{
			if (nDropshipBayCameos > 0)
			{
				// Find in what Dropship is located
				int nDropship = (command - btn_BasicDropshipCameo_ID) / nDropshipBayCameos;
				int index = command - btn_BasicDropshipCameo_ID - (nDropship * nDropshipBayCameos);

				auto pType = dropshipBayChosenUnitsLists[nDropship][index];

				if (pType)
				{
					dropshipLoadoutMoney += pType->Cost;
					auto& affectedDropship = dropshipBayChosenUnitsLists[nDropship];
					affectedDropship.erase(affectedDropship.begin() + index);
					affectedDropship.push_back(nullptr);
					repaintAll = true;

					// Update unit's count in the dropships
					if (dropshipBayChosenUnitsCount.count(pType) > 0)
						--dropshipBayChosenUnitsCount[pType];
					else
						dropshipBayChosenUnitsCount[pType] = 0;

					// Click sound
					VocClass::PlayGlobal(RulesClass::Instance->SellSound, 0x2000, 1.0);
				}
			}
		}
		else if (isHoveringOverDropshipCameos || isHoveringOverSidebarCameos)
		{
			lastTimeWasOverCameos = true;
			repaintAll = true;
		}
		else if (lastTimeWasOverCameos && !isHoveringOverDropshipCameos && !isHoveringOverSidebarCameos)
		{
			lastTimeWasOverCameos = false;
			repaintAll = true;
		}

		// Ends this logic and starts the game
		if (command == VK_SPACE)
			done = true;

		// Reset all the selected units in dropships
		if (command == VK_ESCAPE)
		{
			bool soldAny = false;
			lastSelected = nullptr;
			dropshipBayChosenUnitsCount.clear();

			for (auto& dropshipBay : dropshipBayChosenUnitsLists)
			{
				for (auto& slot : dropshipBay)
				{
					if (slot != nullptr)
						soldAny = true;

					slot = nullptr;
				}
			}

			dropshipLoadoutMoney = ScenarioExt::Global()->DropshipLoadoutMoney >= 0 ? ScenarioExt::Global()->DropshipLoadoutMoney : HouseClass::CurrentPlayer->Available_Money();

			repaintAll = true;

			if (soldAny) // Click sound
			{
				VocClass::PlayGlobal(RulesClass::Instance->SellSound, 0x2000, 1.0);
			}
		}

		// --- RENDER THE SCREEN ---
		if (repaintAll)
		{
			// Screen background
			CC_Draw_Shape(
				DSurface::Hidden,
				screenBG_pal,
				screenBG_Image,
				0,
				&noLocation,
				&windowRectangle,
				BlitterFlags::None,
				0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0
			);

			// Painting the sidebar cameos
			for (int i = 0; i < nSidebarCameos; i++)
			{
				int newIndex = firstBrowsableCameo + i;
				if (newIndex >= availableUnits.size())
					continue;

				int sidebarCameoID = btn_BasicSidebarCameo_ID + i;

				auto const pType = availableUnits[newIndex];
				int maxInstances = allowableUnitMaximums[newIndex] < 0 ? INT_MAX : allowableUnitMaximums[newIndex];
				int nInstances = dropshipBayChosenUnitsCount.count(pType) > 0 ? dropshipBayChosenUnitsCount[pType] : 0;

				int totalDropshipChosenUnits = 0;
				for (const auto& pair : dropshipBayChosenUnitsCount)
				{
					totalDropshipChosenUnits += pair.second;
				}

				bool dropshipsWithFreeSlots = totalDropshipChosenUnits < totalDropshipSlots;

				BlitterFlags bf = BlitterFlags::None;
				if (nInstances >= maxInstances || !dropshipsWithFreeSlots)
					bf = BlitterFlags::bf_400 | BlitterFlags::Darken;

				/*if (!dropshipsWithFreeSlots)
				{
					// Draw the border that represents the last clicked unit
					RectangleStruct newRectangle = sidebarCameoLocations[i];
					newRectangle.X -= 2;
					newRectangle.Width += 4;

					auto foreColor = ColorStruct { 255, 0, 0 }; // By default is red
					int opacity = 255; // Full opacity
					DSurface::Hidden->FillRectTrans(&newRectangle, &foreColor, opacity);
				}
				else*/ if (isHoveringOverSidebarCameos && buttonID == sidebarCameoID && validSidebarCameoPurchase)
				{
					auto foreColor = ColorStruct { 0, 255, 0 }; // Not valid by default is green

					//if (!validSidebarCameoPurchase)
						//foreColor = ColorStruct { 0, 0, 0 }; // Not valid by default is red

					// Draw the border that represents the "Buy" operation in the sidebar
					RectangleStruct newRectangle = sidebarCameoLocations[i];
					newRectangle.X -= 2;
					newRectangle.Width += 4;

					int opacity = 255; // Full opacity

					//if (!validSidebarCameoPurchase)
						//opacity = 0; // Invisible

					DSurface::Hidden->FillRectTrans(&newRectangle, &foreColor, opacity);
				}
				else if (pType == lastSelected)
				{
					// Draw the border that represents the last clicked unit
					RectangleStruct newRectangle = sidebarCameoLocations[i];
					newRectangle.X -= 2;
					newRectangle.Width += 4;

					auto foreColor = ColorStruct { 255, 239, 99 }; // By default is yellow
					int opacity = 255; // Full opacity
					DSurface::Hidden->FillRectTrans(&newRectangle, &foreColor, opacity);
				}

				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
				auto const pPCXSurface = pTypeExt->CameoPCX.GetSurface();
				auto pFileSHP = pType->Cameo;
				auto pPalette = FileSystem::CAMEO_PAL;

				DrawImage(
					DSurface::Hidden,
					sidebarCameoLocations[i],
					pPCXSurface,
					pFileSHP,
					pPalette,
					0,
					-2,
					bf
				);
			}

			// Painting the UP arrow
			CC_Draw_Shape(
				DSurface::Hidden,
				screenBG_pal,
				upArrow_Image,
				0,
				&noLocation,
				&upArrowLocation,
				BlitterFlags::None,
				0, -2, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0
			);

			// Painting the DOWN arrow
			CC_Draw_Shape(
				DSurface::Hidden,
				screenBG_pal,
				downArrow_Image,
				0,
				&noLocation,
				&downArrowLocation,
				BlitterFlags::None,
				0, -2, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0
			);

			// Painting all the Dropship slot cameos
			for (int i = 0; i < dropshipBayCameoLocations.size(); i++)
			{
				for (int j = 0; j < dropshipBayCameoLocations[i].size(); j++)
				{
					auto const pType = dropshipBayChosenUnitsLists[i][j];
					if (!pType)
						continue;

					if (isHoveringOverDropshipCameos && mouseLocationInDropshipCameos.X == i && mouseLocationInDropshipCameos.Y == j)
					{
						// Draw the border that represents the "remove" operation in the sidebar
						RectangleStruct newRectangle = dropshipBayCameoLocations[mouseLocationInDropshipCameos.X][mouseLocationInDropshipCameos.Y];
						newRectangle.X -= 2;
						newRectangle.Width += 4;

						auto foreColor = ColorStruct { 255, 0, 0 }; // By default is red
						int opacity = 255; // Full opacity
						DSurface::Hidden->FillRectTrans(&newRectangle, &foreColor, opacity);
					}
					
					auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
					auto const pPCXSurface = pTypeExt->CameoPCX.GetSurface();
					auto pFileSHP = pType->Cameo;
					auto pPalette = FileSystem::CAMEO_PAL;

					DrawImage(
						DSurface::Hidden,
						dropshipBayCameoLocations[i][j],
						pPCXSurface,
						pFileSHP,
						pPalette,
						0,
						-2
					);
				}
			}

			// Paint the remaining money
			wchar_t buffer[64];
			swprintf_s(buffer, L"Credits: %d", dropshipLoadoutMoney);
			COLORREF foreColor = Drawing::RGB_To_Int(255, 239, 99);
			TextPrintType style = (TextPrintType::FullShadow | TextPrintType::Point6Grad);
			Point2D creditsLabel = {
				windowRectangle.Width - 140,
				windowRectangle.Height - 15
			};
			DSurface::Hidden->DrawTextA(buffer, &windowRectangle, &creditsLabel, foreColor, 0, style);

			// Paint the helper message for starting the mission
			swprintf_s(buffer, L"Press SPACE to start the mission");
			foreColor = Drawing::RGB_To_Int(255, 255, 255);
			style = (TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point6Grad);
			Point2D pressSpaceLabel = {
				(windowRectangle.Width - 175) / 2,
				windowRectangle.Height - 15
			};
			DSurface::Hidden->DrawTextA(buffer, &windowRectangle, &pressSpaceLabel, foreColor, 0, style);

			repaintAll = false;
		}

		GScreenClass::Instance.DoBlit(true, DSurface::Hidden, nullptr);
	}
	// --- MAIN LOOP END ---


	// --- CLEANUP ---
	for (auto button : buttonsList)
	{
		GameDelete(button);
	}

	buttonsList.clear();
	//dropshipBayCameoLocations.clear();
	//buttonsList.clear();
	//GameDelete(commandManager);

	free(file_Background);
	free(file_Loadout);
	free(file_PilotLit);
	free(file_NotAvailableIcon);
	free(file_UpArrow);
	free(file_DownArrow);
	free(file_DropshipPalette);
	free(file_CameoPalette);
	//free(file_Pilot_Green);

	GameDelete(screenBG_Image);
	GameDelete(screenBG_pal);

	// --- EXIT ---
	return EndFunction;
}
