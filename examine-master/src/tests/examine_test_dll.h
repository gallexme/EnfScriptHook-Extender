/* Examine - a tool for memory leak detection on Windows
 *
 * Copyright (C) 2012-2013 Vincent Torri.
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXAMINE_TEST_DLL_H
#define EXAMINE_TEST_DLL_H

#ifdef _WIN32
# define EXM_DLL_EXPORT __declspec(dllexport)
# else
# define EXM_DLL_EXPORT
#endif

EXM_DLL_EXPORT void examine_test_dll(void);

#endif /* EXAMINE_TEST_DLL_H */
