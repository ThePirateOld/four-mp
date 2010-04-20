/// \file
/// \brief Header file that describes the core console commands.
/// \details This file describes only command callbacks. They all have syntax:
/// void func(const ConsoleCore *concore, unsigned char numargs);
/// \author FaTony

#pragma once

#include "ConsoleCore.h"

/// \brief Show the list of convars/concommands.
void ConCmdCvarlist(ConsoleCore *concore, unsigned char numargs);

/// \brief Find concommands with the specified string in their name/help text.
void ConCmdFind(ConsoleCore *concore, unsigned char numargs);

/// \brief Find help about a convar/concommand.
void ConCmdHelp(ConsoleCore *concore, unsigned char numargs);