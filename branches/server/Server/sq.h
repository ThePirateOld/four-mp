/*
*	Created:			28.10.09
*	Created by:			WNeZRoS
*	Last Modifed:		-
*/

#include "sq\squirrel.h"
#include "sq\sqstdsystem.h"

// SQ Functions
SQInteger register_global_func(HSQUIRRELVM v,SQFUNCTION f,const char *fname);

/// \brief Sets the virtual machine name.
void sq_SetScriptName(HSQUIRRELVM v);

/// \brief Sets the virtual machine version.
void sq_SetScriptVersion(HSQUIRRELVM v);

/// \brief Sets the virtual machine author.
void sq_SetScriptAuthor(HSQUIRRELVM v);
// SQ Callbacks

/// \brief Called when the game mode starts.
void sc_OnGameModeInit(HSQUIRRELVM v);

/// \brief Called when the game mode ends.
void sc_OnGameModeExit(HSQUIRRELVM v);

/// \brief Called when the filterscript is loaded.
void sc_OnFilterScriptInit(HSQUIRRELVM v);

/// \brief Called when the filterscript is unloaded.
void sc_OnFilterScriptExit(HSQUIRRELVM v);

/// \brief Called when a player connects the server.
/// \param[in] playerid TODO:
/// \param[in] name TODO:
/// \return TODO:
int sc_OnPlayerConnect(HSQUIRRELVM v, int playerid, char name[32]);

/// \brief Called when a player is disconnecting from the server.
/// \param[in] playerid TODO:
/// \return No return.
void sc_OnPlayerDisconnect(HSQUIRRELVM v, int playerid);

/// \brief Called when a player is spawning.
/// \param[in] playerid TODO:
/// \param[in] cl TODO:
/// \return No return.
void sc_OnPlayerSpawn(HSQUIRRELVM v, int playerid, int);

/// \brief Fires a dynamic command callback.
/// \param[in] callback Name of the callback to call.
/// \param[in] numargs Number of arguments to pass.
/// \return No return.
void sc_CommandCallback(HSQUIRRELVM v, const char *callback, const unsigned char numargs);
// SQ Script Functions
void sq_CreateCar(HSQUIRRELVM v);
void sq_GiveWeapon(HSQUIRRELVM v);
void sq_addPlayerClass(HSQUIRRELVM v);
void sq_enableComponentSelect(HSQUIRRELVM v);