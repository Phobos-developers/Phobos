#pragma once

// Interop exports for accessing scenario local/global variables.
// Phobos extends the vanilla variable value range from char (0/1) to int,
// and exposes them through these functions for external engine extensions.

#include "Utilities/Macro.h"

/// <summary>
/// Gets the value of a local variable by index.
/// </summary>
/// <param name="index">Variable index</param>
/// <param name="pValue">Receives the variable value. Set to 0 if the variable does not exist.</param>
/// <returns>S_OK if the variable exists, S_FALSE if it does not, E_POINTER if pValue is null, E_FAIL if ScenarioExt is not initialized.</returns>
DEFINE_EXPORT(HRESULT, Variables_GetLocal_Phobos, int index, int* pValue);

/// <summary>
/// Sets the value of a local variable by index. Creates the variable if it does not exist.
/// </summary>
/// <param name="index">Variable index</param>
/// <param name="value">New value for the variable</param>
/// <returns>S_OK on success, E_FAIL if ScenarioExt is not initialized.</returns>
DEFINE_EXPORT(HRESULT, Variables_SetLocal_Phobos, int index, int value);

/// <summary>
/// Gets the value of a global variable by index.
/// </summary>
/// <param name="index">Variable index</param>
/// <param name="pValue">Receives the variable value. Set to 0 if the variable does not exist.</param>
/// <returns>S_OK if the variable exists, S_FALSE if it does not, E_POINTER if pValue is null, E_FAIL if ScenarioExt is not initialized.</returns>
DEFINE_EXPORT(HRESULT, Variables_GetGlobal_Phobos, int index, int* pValue);

/// <summary>
/// Sets the value of a global variable by index. Creates the variable if it does not exist.
/// </summary>
/// <param name="index">Variable index</param>
/// <param name="value">New value for the variable</param>
/// <returns>S_OK on success, E_FAIL if ScenarioExt is not initialized.</returns>
DEFINE_EXPORT(HRESULT, Variables_SetGlobal_Phobos, int index, int value);
