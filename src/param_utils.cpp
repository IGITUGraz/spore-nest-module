/*
 * This file is part of SPORE.
 *
 * Copyright (C) 2016, the SPORE team (see AUTHORS).
 *
 * SPORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * SPORE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SPORE.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For more information see: https://github.com/IGITUGraz/spore-nest-module
 *
 * File:   param_utils.cpp
 * Author: kappel
 *
 * Created on March 31, 2017, 6:10 PM
 */

#include "param_utils.h"

namespace spore
{

/**
 * Constructor.
 */
SetDefault::SetDefault()
{
}


/**
 * Constructor.
 */
GetStatus::GetStatus(DictionaryDatum & d)
: d_(d)
{
}


/**
 * Constructor.
 */
CheckParameters::CheckParameters(const DictionaryDatum & d)
: d_(d)
{
}


/**
 * Constructor.
 */
SetStatus::SetStatus(const DictionaryDatum & d)
: d_(d)
{
}


}
