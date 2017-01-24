/* 
 * File:   connection_data_logger.h
 * Author: Kappel
 *
 * Created on November 8, 2016, 11:57 AM
 */

#ifndef CONNECTION_DATA_LOGGER_H
#define	CONNECTION_DATA_LOGGER_H

#include <string>
#include <vector>

#include "nest.h"
#include "dictdatum.h"


namespace spore
{

/**
 * Base class to all data loggers for connections.
 */
class ConnectionDataLoggerBase
{
public:
    typedef nest::index recorder_port;
    
    ConnectionDataLoggerBase();
    ~ConnectionDataLoggerBase();
    
    void get_status(DictionaryDatum &d, recorder_port port) const;
    void set_status(const DictionaryDatum &d, recorder_port &port);
    void clear();
    
protected:
    /**
     * Data structure that holds the recorded data.
     */    
    struct RecorderData
    {
        RecorderData(size_t size);
        void clear();
        
        std::vector<double> recorder_times_;
        std::vector< std::vector<double> > recorder_values_;
        double interval_;
    };
    
    /**
     * Data structure that holds information about variables.
     */    
    struct RecorderInfo
    {
        RecorderInfo(std::string name)
                :name_(name)             
                {};                
        
        std::string name_;
    };

    recorder_port add_recordable_connection();
    
    std::vector<RecorderData*> recorder_data_;
    std::vector<RecorderInfo> recorder_info_;
};


/**
 * Generic version of data logger for connections.
 */
template<typename ConnectionType>
class ConnectionDataLogger : public ConnectionDataLoggerBase
{
public:
    typedef double ( ConnectionType::*DataAccessFct )() const;
    
    void register_recordable_variable( const std::string &name, DataAccessFct data_access_fct );
    void record( double time, ConnectionType const &host, recorder_port port );

private:
    std::vector<DataAccessFct> data_access_fct_;
};


//
// ConnectionDataLogger implementation.
//


/**
 * Add a new recordable variable to the recorder object.
 * 
 * @param name name of the recordable variable.
 * @param data_access_fct pointer to the member function to retrieve the variable.
 */
template<typename ConnectionType>
void ConnectionDataLogger<ConnectionType>::register_recordable_variable( const std::string &name,
                                                                         DataAccessFct data_access_fct )
{
    recorder_info_.push_back( RecorderInfo(name) );
    data_access_fct_.push_back(data_access_fct);
}


/**
 * Record current variable values from the given host connection. Values are
 * recorded only if the last recording time is older than the recorder interval.
 * 
 * @param time current time of recording.
 * @param host the host connection.
 * @param port the recorder port of the host connection.
 */
template<typename ConnectionType>
void ConnectionDataLogger<ConnectionType>::record( double time_step,
                                                   ConnectionType const &host,
                                                   recorder_port port )
{
    if (port == nest::invalid_index)
        return;
    
    assert( port <  recorder_data_.size() );
    ConnectionDataLoggerBase::RecorderData &recorder = *recorder_data_[port];
    
    if (recorder.interval_==0)
        return;
        
    if ( recorder.recorder_times_.empty() || (recorder.recorder_times_.back()+recorder.interval_<=time_step) )
    {
        recorder.recorder_times_.push_back(time_step);
        
        for (int i=0; i<recorder.recorder_values_.size(); i++)
        {
            recorder.recorder_values_[i].push_back( ( (host).*( data_access_fct_[i] ) )() );
        }
    }
}

}

#endif	/* CONNECTION_DATA_LOGGER_H */

