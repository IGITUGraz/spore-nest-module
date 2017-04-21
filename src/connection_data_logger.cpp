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
 * File:   connection_data_logger.cpp
 * Author: Kappel
 *
 * Created on November 8, 2016, 11:57 AM
 */

#include "connection_data_logger.h"

#include "dictutils.h"
#include "exceptions.h"


namespace spore
{

/**
 * Constructor.
 */
ConnectionDataLoggerBase::ConnectionDataLoggerBase()
{
}

/**
 * Destructor.
 */
ConnectionDataLoggerBase::~ConnectionDataLoggerBase()
{
    for (std::vector<RecorderData*>::iterator it = recorder_data_.begin();
            it != recorder_data_.end();
            ++it)
    {
        delete *it;
    }
}

/**
 * Get recorded values of recorder at given port.
 * 
 * @param d dictionary to retrieve data.
 * @param port recorder port of the calling connection.
 */
void ConnectionDataLoggerBase::get_status(DictionaryDatum &d, recorder_port port) const
{
    if (port == nest::invalid_index)
        return;

    assert(port < recorder_data_.size());
    ConnectionDataLoggerBase::RecorderData &recorder = *recorder_data_[port];

    (*d)["recorder_times"] = recorder.recorder_times_;

    for (size_t i = 0; i < recorder_info_.size(); i++)
    {
        (*d)[recorder_info_[i].name_ + "_values"] = recorder.recorder_values_[i];
    }
}

/**
 * Set the status of the recorder at given port. The recorder port may
 * change to a new value during this call.
 * 
 * @param d dictionary of parameter values.
 * @param port recorder port of the calling connection.
 */
void ConnectionDataLoggerBase::set_status(const DictionaryDatum &d, recorder_port &port)
{
    double interval = 0.0;
    updateValue<double>(d, "recorder_interval", interval);

    if (interval > 0.0 && (port == nest::invalid_index))
        port = add_recordable_connection();

    if (port == nest::invalid_index)
        return;

    assert(port < recorder_data_.size());

    recorder_data_[port]->interval_ = interval;

    bool reset_recorder = false;
    updateValue<bool>(d, "reset_recorder", reset_recorder);

    if (reset_recorder)
    {
        recorder_data_[port]->clear();
    }
}

/**
 * Add new recordable connection to the recorder.
 * 
 * @return the port id of the new recorder.
 */
ConnectionDataLoggerBase::recorder_port ConnectionDataLoggerBase::add_recordable_connection()
{
    if (recorder_data_.size() == nest::invalid_index)
        throw nest::BadProperty("Maximum number of recorders reached.");

    recorder_data_.push_back(new RecorderData(recorder_info_.size()));
    return recorder_data_.size() - 1;
}

/**
 * Clears all recorded data.
 */
void ConnectionDataLoggerBase::clear()
{
    for (std::vector<RecorderData*>::iterator it = recorder_data_.begin();
            it != recorder_data_.end();
            ++it)
    {
        (*it)->clear();
    }
}

//
// ConnectionDataLoggerBase::RecorderData implementation.
//

/**
 * Constructor.
 * 
 * @param size number of variables in the recorder buffer.
 */
ConnectionDataLoggerBase::RecorderData::RecorderData(size_t size)
: interval_(0.0)
{
    recorder_values_.resize(size);
}

/**
 * Constructor.
 * 
 * @param size number of variables in the recorder buffer.
 */
void ConnectionDataLoggerBase::RecorderData::clear()
{
    recorder_times_.clear();

    for (std::vector< std::vector<double> >::iterator it = recorder_values_.begin();
            it != recorder_values_.end();
            ++it)
    {
        it->clear();
    }
}

}

