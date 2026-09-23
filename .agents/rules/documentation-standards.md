# Phobos Documentation Standards

When documenting features in Phobos (`docs/` directory: `New-or-Enhanced-Logics.md`, `User-Interface.md`, `AI-Scripting-and-Mapping.md`, etc.), strictly adhere to the following standards:

## 1. Feature Explanations (Bullet Points)
- **No Defaults in Explanations**: Do NOT include `(defaults to ...)` or default values inside bullet descriptions. Explanations describe only the tag's functionality, syntax, parameters, and consequences. Default values belong solely in the INI code blocks.
- **SHP before PCX**: When documenting visual assets supporting both SHP and PCX formats, always list the base engine `.SHP` tag first, followed immediately by the `.PCX` counterpart, explicitly noting that PCX takes precedence over SHP. Mention pattern formatting (e.g. `%d`) in the PCX description when supported.
- **Separation of Concerns**: Keep count tags and location/coordinate tags separate. Keep individual UI button or cameo location tags in distinct bullets.
- **No Redundant Index Examples**: When specifying index bases like `(0-based)`, do NOT append redundant examples of the tag name (e.g. avoid `(0-based, e.g. Tag0)`). Simply specify `(0-based)`.

## 2. INI Code Blocks
- **Explicit Default Values**: Match the C++ source code implementation exactly (`Tag=default_value`). If the C++ code has a fallback or default value (including files like `DROPUP.SHP`, `DROPSHIP.PAL` or coordinates like `45,2`), document that exact default on the tag. Empty tags (`Tag=`) are reserved strictly for nullable/optional tags with no default.
- **Clean Semicolon Comments**: After the semicolon `;`, strictly document the data type (e.g., `; boolean`, `; integer`, `; coordinate pair (X,Y)`, `; filename (.shp)`, `; Sound, default to [AudioVisual] -> GenericClick`).
- **No Inline Examples**: NEVER write inline example comments or informal explanations in the code block (e.g., `; e.g. DropshipLoadout.CameoLocation0=55,69` is prohibited).
- **Section Headers**: Include the section object type comment next to headers (e.g., `[SOMECOUNTRY] ; HouseType`, `[SOMESW] ; SuperWeaponType`).
