
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
#include <Ext/House/Body.h>

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

		Point2D noLocation = { 0, 0 };

		CC_Draw_Shape(
			pSurface,
			pPalette,
			fileSHP,
			frameIndex,
			&noLocation,
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
	DSurface* pSurface = DSurface::Hidden;
	pSurface->Fill(0);

	// --- FILENAME INITIALIZATION ---
	// If exists the PCX file then this image format takes precedence
	ConvertClass* dropshipLoadout_Palette = nullptr;
	SHPStruct* dropshipLoadout_Background = nullptr;
	SHPStruct* dropshipLoadout_UpArrow = nullptr;
	SHPStruct* dropshipLoadout_DownArrow = nullptr;
	SHPStruct* dropshipLoadout_Loadout = nullptr;
	SHPStruct* dropshipLoadout_PilotLit = nullptr;
	std::vector<SHPStruct*> dropshipLoadout_DGreenList;
	BSurface* dropshipLoadout_BackgroundPCX = nullptr;
	BSurface* dropshipLoadout_UpArrowPCX = nullptr;
	BSurface* dropshipLoadout_DownArrowPCX = nullptr;
	std::vector<BSurface*> dropshipLoadout_LoadoutPCX;
	std::vector<BSurface*> dropshipLoadout_PilotLitPCX;
	std::vector<BSurface*> dropshipLoadout_DGreenListPCX;

	if (ScenarioExt::Global()->DropshipLoadout_Palette)
		dropshipLoadout_Palette = ScenarioExt::Global()->DropshipLoadout_Palette;
	else
		dropshipLoadout_Palette = FileSystem::LoadPALFile("DROPSHIP.PAL", DSurface::Hidden);

	if (ScenarioExt::Global()->DropshipLoadout_BackgroundPCX.Exists())
		dropshipLoadout_BackgroundPCX = ScenarioExt::Global()->DropshipLoadout_BackgroundPCX.GetSurface();

	if (ScenarioExt::Global()->DropshipLoadout_Background)
	{
		dropshipLoadout_Background = ScenarioExt::Global()->DropshipLoadout_Background;
	}
	else
	{
		char tempFilenameBuffer[32];
		_snprintf_s(tempFilenameBuffer, sizeof(tempFilenameBuffer), "DROP%04d.SHP", nStartingDropships);
		dropshipLoadout_Background = FileSystem::LoadSHPFile(_strdup(tempFilenameBuffer));
	}

	if (ScenarioExt::Global()->DropshipLoadout_LoadoutPCX.size() > 0)
	{
		for (auto &pFilePCX : ScenarioExt::Global()->DropshipLoadout_LoadoutPCX)
		{
			dropshipLoadout_LoadoutPCX.push_back(pFilePCX.GetSurface());
		}
	}

	if (ScenarioExt::Global()->DropshipLoadout_Loadout)
		dropshipLoadout_Loadout = ScenarioExt::Global()->DropshipLoadout_Loadout;
	else
		dropshipLoadout_Loadout = FileSystem::LoadSHPFile("LOADOUT.SHP");

	if (ScenarioExt::Global()->DropshipLoadout_PilotLitPCX.size() > 0)
	{
		for (auto &pFilePCX : ScenarioExt::Global()->DropshipLoadout_PilotLitPCX)
		{
			dropshipLoadout_PilotLitPCX.push_back(pFilePCX.GetSurface());
		}
	}

	if (ScenarioExt::Global()->DropshipLoadout_PilotLit)
		dropshipLoadout_PilotLit = ScenarioExt::Global()->DropshipLoadout_PilotLit;
	else
		dropshipLoadout_PilotLit = FileSystem::LoadSHPFile("PILOTLIT.SHP");

	if (ScenarioExt::Global()->DropshipLoadout_UpArrowPCX.Exists())
		dropshipLoadout_UpArrowPCX = ScenarioExt::Global()->DropshipLoadout_UpArrowPCX.GetSurface();

	if (ScenarioExt::Global()->DropshipLoadout_UpArrow)
		dropshipLoadout_UpArrow = ScenarioExt::Global()->DropshipLoadout_UpArrow;
	else
		dropshipLoadout_UpArrow = FileSystem::LoadSHPFile("DROPUP.SHP");

	if (ScenarioExt::Global()->DropshipLoadout_DownArrowPCX.Exists())
		dropshipLoadout_DownArrowPCX = ScenarioExt::Global()->DropshipLoadout_DownArrowPCX.GetSurface();

	if (ScenarioExt::Global()->DropshipLoadout_DownArrow)
		dropshipLoadout_DownArrow = ScenarioExt::Global()->DropshipLoadout_DownArrow;
	else
		dropshipLoadout_DownArrow = FileSystem::LoadSHPFile("DROPDOWN.SHP");

	// The original 4 sidebar row animations
	for (int i = 0; i < 4; i++)
	{
		if (ScenarioExt::Global()->DropshipLoadout_DGreenList.size() < 4
			|| ScenarioExt::Global()->DropshipLoadout_DGreenList[i] == nullptr)
		{
			if (i == 0)
				dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN1.SHP"));
			else if (i == 1)
				dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN2.SHP"));
			else if (i == 2)
				dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN3.SHP"));
			else if (i == 3)
				dropshipLoadout_DGreenList.push_back(FileSystem::LoadSHPFile("DGREEN4.SHP"));
			else
				dropshipLoadout_DGreenList.push_back(nullptr);
		}
		else
		{
			dropshipLoadout_DGreenList.push_back(ScenarioExt::Global()->DropshipLoadout_DGreenList[i]);
		}
	}

	// New rows for sidebar animations (>4)
	for (int i = 4; i < ScenarioExt::Global()->DropshipLoadout_DGreenList.size(); i++)
	{
		dropshipLoadout_DGreenList.push_back(ScenarioExt::Global()->DropshipLoadout_DGreenList[i]);
	}
	
	// --- PRE-LOOP SETUP: MUSIC, MOUSE, AND MONEY ---

	// Initial EVA Voice
	VoxClass::PlayIndex(ScenarioExt::Global()->DropshipLoadout_StartEVA.Get(-1));

	// Play the specific theme for the dropship loadout screen
	const int theme = ScenarioExt::Global()->DropshipLoadout_Theme;
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
	long dropshipLoadout_Money = ScenarioExt::Global()->DropshipLoadout_Money >= 0 ? ScenarioExt::Global()->DropshipLoadout_Money : HouseClass::CurrentPlayer->Available_Money();

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

	// Basic background location data
	int backgroundWidth = 0; // = dropshipLoadout_Background->Width;
	int backgroundHeight = 0; // = dropshipLoadout_Background->Height;

	if (dropshipLoadout_BackgroundPCX)
	{
		backgroundWidth = dropshipLoadout_BackgroundPCX->Width;
		backgroundHeight = dropshipLoadout_BackgroundPCX->Height;
	}
	else
	{
		backgroundWidth = dropshipLoadout_Background->Width;
		backgroundHeight = dropshipLoadout_Background->Height;
	}

	// Calculate the top-left corner coordinates to center the background image.
	int backgroundX = (pSurface->GetWidth() - backgroundWidth) / 2;
	int backgroundY = (pSurface->GetHeight() - backgroundHeight) / 2;
	int screenWidth = backgroundX + backgroundWidth;
	int screenHeight = backgroundY + backgroundHeight;

	// Store the final position and dimensions for later drawing operations.
	RectangleStruct windowRectangle = { backgroundX, backgroundY, backgroundWidth, backgroundHeight };

	// --- 5. CALCULATE UI ELEMENT POSITIONS ---
	// Pre-calculate the screen positions for dynamic UI elements like cameos.

	// Calculate positions for the sidebar cameos
	int nSidebarCameos = 8;
	//int nSidebarRows = 4;
	std::vector<RectangleStruct> sidebarCameoLocations;

	for (int i = 0; i < nSidebarCameos; ++i)
	{
		int cameoX = backgroundX + 493 + 68 * (i % 2);
		int cameoY = backgroundY + 25 + 50 * (i / 2);
		RectangleStruct cameoRectangle = { cameoX, cameoY, cameoWidth, cameoHeight };
		sidebarCameoLocations.push_back(cameoRectangle);
	}

	// Calculate positions for the scroll arrow buttons

	// Center point between the two cameo columns.
	int centerOfCameoColumns = sidebarCameoLocations[0].X + sidebarCameoLocations[0].Width + (sidebarCameoLocations[1].X - (sidebarCameoLocations[0].X + sidebarCameoLocations[0].Width)) / 2;

	// Y position below the last row of cameos
	int arrowsY = sidebarCameoLocations.back().Y + sidebarCameoLocations.back().Height + 6;

	int dropshipLoadout_UpArrowWidth = dropshipLoadout_UpArrowPCX ? dropshipLoadout_UpArrowPCX->Width : dropshipLoadout_UpArrow->Width;
	int dropshipLoadout_UpArrowHeight = dropshipLoadout_UpArrowPCX ? dropshipLoadout_UpArrowPCX->Height : dropshipLoadout_UpArrow->Height;

	RectangleStruct upArrowLocation = {
		centerOfCameoColumns - dropshipLoadout_UpArrowWidth, // Position left of center
		arrowsY,
		dropshipLoadout_UpArrowWidth,
		dropshipLoadout_UpArrowHeight
	};

	int dropshipLoadout_DownArrowWidth = dropshipLoadout_DownArrowPCX ? dropshipLoadout_DownArrowPCX->Width : dropshipLoadout_DownArrow->Width;
	int dropshipLoadout_DownArrowHeight = dropshipLoadout_DownArrowPCX ? dropshipLoadout_DownArrowPCX->Height : dropshipLoadout_DownArrow->Height;

	RectangleStruct downArrowLocation = {
		centerOfCameoColumns, // Position right of center
		arrowsY,
		dropshipLoadout_DownArrowWidth,
		dropshipLoadout_DownArrowHeight
	};

	std::vector<RectangleStruct> dGreenLocation;
	int dGreenX = 371;
	int dGreenY = 10;

	for (auto dGreen : dropshipLoadout_DGreenList)
	{
		if (!dGreen) // Invalid graphics
		{
			dGreenLocation.push_back({ 0, 0, 0, 0 });
			continue;
		}

		RectangleStruct dGreenRectangle = {
			backgroundX + dGreenX,
			backgroundY + dGreenY,
			dGreen->Width,
			dGreen->Height
		};

		dGreenY += 50;
		dGreenLocation.push_back(dGreenRectangle);
	}

	int dropshipLoadout_LoadoutWidth = dropshipLoadout_LoadoutPCX.size() > 0 ? dropshipLoadout_LoadoutPCX[0]->Width : dropshipLoadout_Loadout->Width;
	int dropshipLoadout_LoadoutHeight = dropshipLoadout_LoadoutPCX.size() > 0 ? dropshipLoadout_LoadoutPCX[0]->Height : dropshipLoadout_Loadout->Height;

	RectangleStruct loadoutLocation = {
		backgroundX + 45,
		backgroundY + 2, // Note: "+0" for a perfect alignment with TS values but the right background is "+2"...
		dropshipLoadout_LoadoutWidth,
		dropshipLoadout_LoadoutHeight
	};

	int dropshipLoadout_PilotLitWidth = dropshipLoadout_PilotLitPCX.size() > 0 ? dropshipLoadout_PilotLitPCX[0]->Width : dropshipLoadout_PilotLit->Width;
	int dropshipLoadout_PilotLitHeight = dropshipLoadout_PilotLitPCX.size() > 0 ? dropshipLoadout_PilotLitPCX[0]->Height : dropshipLoadout_PilotLit->Height;

	RectangleStruct pilotLitLocation = {
		backgroundX + 284,
		backgroundY + 151,
		dropshipLoadout_PilotLitWidth,
		dropshipLoadout_PilotLitHeight
	};

	// Calculate positions for the dropship slots
	int nDropshipBayCameos = 5;
	int nDropshipBayTotalSlots = nStartingDropships * nDropshipBayCameos;
	std::vector<std::vector<RectangleStruct>> dropshipBayCameoLocations;

	// Case 1: Slots coordinates of the Dropship #1
	if (nStartingDropships == 1 || nStartingDropships == 2)
	{
		int cameoX = backgroundX + 55;
		int cameoY = backgroundY + 69;
		std::vector<RectangleStruct> dropshipBayCameoLocationsList;

		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 132, cameoY + 50, cameoWidth, cameoHeight });

		dropshipBayCameoLocations.push_back(dropshipBayCameoLocationsList);
		dropshipBayCameoLocationsList.clear();
	}

	// Case 2: Slots coordinates of the Dropship #2
	if (nStartingDropships == 2)
	{
		int cameoX = backgroundX + 55;
		int cameoY = backgroundY + 209;
		std::vector<RectangleStruct> dropshipBayCameoLocationsList;

		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 132, cameoY + 50, cameoWidth, cameoHeight });

		dropshipBayCameoLocations.push_back(dropshipBayCameoLocationsList);
		dropshipBayCameoLocationsList.clear();
	}

	// Case 3: Slots coordinates of the Dropship #3
	if (nStartingDropships == 3)
	{
		int cameoX = backgroundX + 55;
		int cameoY = backgroundY + 39;
		std::vector<RectangleStruct> dropshipBayCameoLocationsList;

		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 132, cameoY + 50, cameoWidth, cameoHeight });

		dropshipBayCameoLocations.push_back(dropshipBayCameoLocationsList);
		dropshipBayCameoLocationsList.clear();

		cameoY += 120;
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 132, cameoY + 50, cameoWidth, cameoHeight });

		dropshipBayCameoLocations.push_back(dropshipBayCameoLocationsList);
		dropshipBayCameoLocationsList.clear();

		cameoY += 120;
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 66, cameoY + 50, cameoWidth, cameoHeight });
		dropshipBayCameoLocationsList.push_back({ cameoX + 132, cameoY + 50, cameoWidth, cameoHeight });

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
	bool pressedSpaceKey = false;
	bool repaintAll = true; // Force initial draw
	int firstBrowsableCameo = 0; // Points to the first element in the sidebar to be drawed. Arrows modify this index

	bool lastTimeWasOverCameos = false;
	int totalDropshipSlots = nStartingDropships * nDropshipBayCameos;

	commandManager->TurnOn();

	// Animations setup
	int currentLoadoutFrame = -1; // No image
	int currentPilotLitFrame = -1; // No image
	int loadoutFrameDelay = 11;
	int pilotLitFrameDelay = 15;
	int loadoutTotalFrames = dropshipLoadout_LoadoutPCX.size() > 0 ? dropshipLoadout_LoadoutPCX.size() - 1 : dropshipLoadout_Loadout->Frames;
	int pilotLitTotalFrames = dropshipLoadout_PilotLitPCX.size() > 0 ? dropshipLoadout_PilotLitPCX.size() - 1 : dropshipLoadout_PilotLit->Frames;

	int animTimer_StartValue = 15; // By default a frame update is completed every 15ms
	int animTimer_DelayedStartValue_Loadout = ScenarioClass::Instance->Random(0, 0); // Disabled by default to resemble the original but it will be customizable for modders
	int animTimer_DelayedStartValue_PilotLit = ScenarioClass::Instance->Random(100, 300);

	SysTimerClass animTimer_UpdateFrameTimer;
	SysTimerClass animTimer_DelayedStartTimer_Loadout; // Delay before the frame updater starts
	SysTimerClass animTimer_UpdateFrameTimer_Loadout;
	SysTimerClass animTimer_DelayedStartTimer_PilotLit; // Delay before the frame updater starts
	SysTimerClass animTimer_UpdateFrameTimer_PilotLit;

	animTimer_DelayedStartTimer_Loadout.Start(animTimer_DelayedStartValue_Loadout);
	animTimer_DelayedStartTimer_PilotLit.Start(animTimer_DelayedStartValue_PilotLit);
	animTimer_UpdateFrameTimer_Loadout.Start(loadoutFrameDelay);
	animTimer_UpdateFrameTimer_PilotLit.Start(pilotLitFrameDelay);

	int sidebarRowAnimationIndex = -1; // By default is DGREENx.SHP being x=[0-3]
	int currentSidebarRowAnimationFrame = 0;
	int sidebarRowAnimationFrameDelay = 5;
	int sidebarRowAnimationTotalFrames = sidebarRowAnimationIndex >= 0 ? dropshipLoadout_DGreenList[sidebarRowAnimationIndex]->Frames : 0;
	SysTimerClass animTimer_UpdateFrameTimer_SidebarRowAnimation;

	while (!pressedSpaceKey)
	{
		Game::CallBack();

		// Get any input
		int command = commandManager->Input();

		int buttonID = -1;
		// Check if a mouse click has happened inside a button.
		// If so, it overrides any keyboard command from this frame
		RectangleStruct mouseRect = WWMouseClass::Instance->Rect2;

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

		// Translating Key/Mouse input values
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
		bool pressedUpArrow = command == VK_UP || ((pressedLeftClick || command == (32768 + btn_ScrollUp_ID)) && isUpArrow);
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

		// Checking if the focused cameo is buyable
		if (isHoveringOverSidebarCameos || pressedAnySidebarCameo)
		{
			int sidebarIndex = firstBrowsableCameo + (buttonID - btn_BasicSidebarCameo_ID);

			if (sidebarIndex < availableUnits.size())
			{
				auto const pType = availableUnits[sidebarIndex];
				int maxInstances = allowableUnitMaximums[sidebarIndex] < 0 ? INT_MAX : allowableUnitMaximums[sidebarIndex];
				int nInstances = dropshipBayChosenUnitsCount.count(pType) > 0 ? dropshipBayChosenUnitsCount[pType] : 0;

				if (nInstances < maxInstances
					&& pType->Cost <= dropshipLoadout_Money
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

		// Execute the right button action with the pressed key:

		// Check if pressed UP or DOWN keys and update the index of the first element to be showed in the sidebar
		if (pressedUpArrow) // UP key or click in the UP button
		{
			if (firstBrowsableCameo >= 2)
			{
				firstBrowsableCameo -= 2;
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
						dropshipLoadout_Money += pType->Cost;

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
						dropshipLoadout_Money -= pType->Cost;
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

				// Find the row's cameos for starting the respective dGreen animation, if isn't running (only 1 instance allowed in the same row)
				if (sidebarRowAnimationIndex < 0)
				{
					sidebarRowAnimationIndex = ((command - btn_BasicSidebarCameo_ID) / 2);

					if (sidebarRowAnimationIndex < dropshipLoadout_DGreenList.size())
						animTimer_UpdateFrameTimer_SidebarRowAnimation.Start(sidebarRowAnimationFrameDelay);
					else
						sidebarRowAnimationIndex = -1; // No images => No animation

					sidebarRowAnimationTotalFrames = sidebarRowAnimationIndex >= 0 ? dropshipLoadout_DGreenList[sidebarRowAnimationIndex]->Frames : 0;
				}
			}
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
					dropshipLoadout_Money += pType->Cost;
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

		// Ends this window and starts the game
		if (command == VK_SPACE)
			pressedSpaceKey = true;

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

			// Restore initial money
			dropshipLoadout_Money = ScenarioExt::Global()->DropshipLoadout_Money >= 0 ? ScenarioExt::Global()->DropshipLoadout_Money : HouseClass::CurrentPlayer->Available_Money();

			repaintAll = true;

			if (soldAny) // Click sound
				VocClass::PlayGlobal(RulesClass::Instance->SellSound, 0x2000, 1.0);
		}

		// --- ANIMATION LOGIC ---

		// Animation 1: LOADOUT.SHP
		if (animTimer_DelayedStartTimer_Loadout.Completed())
		{
			if (animTimer_UpdateFrameTimer_Loadout.Completed())
			{
				if (currentLoadoutFrame < loadoutTotalFrames)
				{
					currentLoadoutFrame++;
				}
				else
				{
					currentLoadoutFrame = -1;
					animTimer_DelayedStartValue_Loadout = ScenarioClass::Instance->Random(0, 0);
					animTimer_DelayedStartTimer_Loadout.Start(animTimer_DelayedStartValue_Loadout);
				}

				animTimer_UpdateFrameTimer_Loadout.Start(loadoutFrameDelay);
				repaintAll = true;
			}
		}

		// Animation 2: PILOTLIT.SHP
		if (animTimer_DelayedStartTimer_PilotLit.Completed())
		{
			if (animTimer_UpdateFrameTimer_PilotLit.Completed())
			{
				if (currentPilotLitFrame < pilotLitTotalFrames)
					currentPilotLitFrame++;
				else
				{
					currentPilotLitFrame = -1;
					animTimer_DelayedStartValue_PilotLit = ScenarioClass::Instance->Random(100, 300);
					animTimer_DelayedStartTimer_PilotLit.Start(animTimer_DelayedStartValue_PilotLit);
				}

				animTimer_UpdateFrameTimer_PilotLit.Start(pilotLitFrameDelay);
				repaintAll = true;
			}
		}

		// Animation 3: dGreen "x" from the list (DGREENx.SHP)
		if (sidebarRowAnimationIndex >= 0)
		{
			if (animTimer_UpdateFrameTimer_SidebarRowAnimation.Completed())
			{
				if (currentSidebarRowAnimationFrame < sidebarRowAnimationTotalFrames)
				{
					currentSidebarRowAnimationFrame++;
					animTimer_UpdateFrameTimer_SidebarRowAnimation.Start(sidebarRowAnimationFrameDelay);
				}
				else
				{
					currentSidebarRowAnimationFrame = -1;
					sidebarRowAnimationIndex = -1;
				}

				repaintAll = true;
			}
		}

		// Restart the animations frame update timer
		if (animTimer_UpdateFrameTimer.Completed())
			animTimer_UpdateFrameTimer.Start(animTimer_StartValue);

		// --- RENDER THE SCREEN ---

		if (repaintAll)
		{
			// Screen background
			DrawImage(
					pSurface,
					windowRectangle,
					dropshipLoadout_BackgroundPCX,
					dropshipLoadout_Background,
					dropshipLoadout_Palette
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

				if (isHoveringOverSidebarCameos
					&& buttonID == sidebarCameoID
					&& validSidebarCameoPurchase)
				{
					auto foreColor = ColorStruct { 0, 255, 0 }; // Not valid by default is green

					// Draw the border that represents the "Buy" operation in the sidebar
					RectangleStruct newRectangle = sidebarCameoLocations[i];
					newRectangle.X -= 2;
					newRectangle.Width += 4;

					int opacity = 255; // Full opacity

					pSurface->FillRectTrans(&newRectangle, &foreColor, opacity);
				}
				else if (pType == lastSelected)
				{
					// Draw the border that represents the last clicked unit
					RectangleStruct newRectangle = sidebarCameoLocations[i];
					newRectangle.X -= 2;
					newRectangle.Width += 4;

					auto foreColor = ColorStruct { 255, 239, 99 }; // By default is yellow
					int opacity = 255; // Full opacity
					pSurface->FillRectTrans(&newRectangle, &foreColor, opacity);
				}

				auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
				auto const pPCXSurface = pTypeExt->CameoPCX.GetSurface();
				auto pFileSHP = pType->Cameo;
				auto pPalette = FileSystem::CAMEO_PAL;

				DrawImage(
					pSurface,
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
			DrawImage(
					pSurface,
					upArrowLocation,
					dropshipLoadout_UpArrowPCX,
					dropshipLoadout_UpArrow,
					dropshipLoadout_Palette,
					0,
					-2
			);

			// Painting the DOWN arrow
			DrawImage(
					pSurface,
					downArrowLocation,
					dropshipLoadout_DownArrowPCX,
					dropshipLoadout_DownArrow,
					dropshipLoadout_Palette,
					0,
					-2
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
						pSurface->FillRectTrans(&newRectangle, &foreColor, opacity);
					}
					
					auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
					auto const pPCXSurface = pTypeExt->CameoPCX.GetSurface();
					auto pFileSHP = pType->Cameo;
					auto pPalette = FileSystem::CAMEO_PAL;

					DrawImage(
						pSurface,
						dropshipBayCameoLocations[i][j],
						pPCXSurface,
						pFileSHP,
						pPalette,
						0,
						-2
					);
				}
			}

			// Paint Loadout animation
			if (currentLoadoutFrame >= 0)
			{
				BSurface* framePCX = dropshipLoadout_LoadoutPCX.size() > 0 ?
					dropshipLoadout_LoadoutPCX[currentLoadoutFrame] : nullptr;

				DrawImage(
					pSurface,
					loadoutLocation,
					framePCX,
					dropshipLoadout_Loadout,
					dropshipLoadout_Palette,
					currentLoadoutFrame,
					-2
				);
			}

			// Paint PilotLit animation
			if (currentPilotLitFrame >= 0)
			{
				BSurface* framePCX = dropshipLoadout_PilotLitPCX.size() > 0 ?
					dropshipLoadout_PilotLitPCX[currentPilotLitFrame] : nullptr;

				DrawImage(
					pSurface,
					pilotLitLocation,
					framePCX,
					dropshipLoadout_PilotLit,
					dropshipLoadout_Palette,
					currentPilotLitFrame,
					-2
				);
			}

			// Paint dGreen animation
			if (sidebarRowAnimationIndex >= 0)
			{
				DrawImage(
					pSurface,
					dGreenLocation[sidebarRowAnimationIndex],
					nullptr,
					dropshipLoadout_DGreenList[sidebarRowAnimationIndex],
					dropshipLoadout_Palette,
					currentSidebarRowAnimationFrame,
					-2
				);
			}

			// Paint the remaining money
			wchar_t buffer[64];
			swprintf_s(buffer, L"Credits: %d", dropshipLoadout_Money);
			COLORREF foreColor = Drawing::RGB_To_Int(255, 239, 99);
			TextPrintType style = (TextPrintType::FullShadow | TextPrintType::Point6Grad);
			Point2D creditsLabel = {
				windowRectangle.Width - 140,
				windowRectangle.Height - 15
			};
			pSurface->DrawTextA(buffer, &windowRectangle, &creditsLabel, foreColor, 0, style);

			// Paint the helper message for starting the mission
			swprintf_s(buffer, L"Press SPACE to start the mission");
			foreColor = Drawing::RGB_To_Int(255, 255, 255);
			style = (TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point6Grad);
			Point2D pressSpaceLabel = {
				(windowRectangle.Width - 175) / 2,
				windowRectangle.Height - 15
			};
			pSurface->DrawTextA(buffer, &windowRectangle, &pressSpaceLabel, foreColor, 0, style);

			repaintAll = false;
		}

		GScreenClass::Instance.DoBlit(true, pSurface, nullptr);
	}
	// --- MAIN LOOP END ---

	// --- SAVE INTO PLAYER DATA ---

	int totalDropshipChosenUnits = 0;

	for (const auto& pair : dropshipBayChosenUnitsCount)
	{
		totalDropshipChosenUnits += pair.second;
	}

	// Fill the smaller number of dropships possible
	auto pHouseExt = HouseExt::ExtMap.Find(HouseClass::CurrentPlayer);
	pHouseExt->DropshipLoadout_Cargo.clear();
	pHouseExt->DropshipLoadout_Carriers.clear();
	int nCarriers = ScenarioExt::Global()->DropshipLoadout_Carriers.size();

	for (int i = 0; i < nStartingDropships && i < nCarriers; i++)
	{
		pHouseExt->DropshipLoadout_Carriers.push_back(ScenarioExt::Global()->DropshipLoadout_Carriers[i]);
		std::vector<TechnoTypeClass*> unitsList;

		// Now fill the transport with the selected units
		for (auto const pTechno : dropshipBayChosenUnitsLists[i])
		{
			if (pTechno)
				unitsList.push_back(pTechno);
		}

		pHouseExt->DropshipLoadout_Cargo.push_back(unitsList);
		unitsList.clear();
	}

	// Update the player's initial money for the mission
	if (ScenarioExt::Global()->DropshipLoadout_AddUnusedMoneyToPlayer)
	{
		HouseClass::CurrentPlayer->TransactMoney(dropshipLoadout_Money);
	}
	else if (ScenarioExt::Global()->DropshipLoadout_Money < 0)
	{
		long spent = HouseClass::CurrentPlayer->Available_Money() - dropshipLoadout_Money;
		HouseClass::CurrentPlayer->TransactMoney(-spent);
	}

	// --- CLEANUP ---

	for (auto button : buttonsList)
	{
		GameDelete(button);
	}

	buttonsList.clear();

	for (auto dGreen : dropshipLoadout_DGreenList)
	{
		GameDelete(dGreen);
	}

	dropshipLoadout_DGreenList.clear();

	//GameDelete(notAvailableIcon_pal);
	GameDelete(dropshipLoadout_Palette);
	GameDelete(dropshipLoadout_BackgroundPCX);
	GameDelete(dropshipLoadout_Background);
	//GameDelete(notAvailableIcon_Image);
	GameDelete(dropshipLoadout_UpArrowPCX);
	GameDelete(dropshipLoadout_UpArrow);
	GameDelete(dropshipLoadout_DownArrowPCX);
	GameDelete(dropshipLoadout_DownArrow);
	GameDelete(dropshipLoadout_Loadout);
	GameDelete(dropshipLoadout_PilotLit);
	//TO-DO
	// DropshipLoadout_LoadoutPCX
	// DropshipLoadout_PilotLitPCX
	// --- EXIT ---
	return EndFunction;
}
