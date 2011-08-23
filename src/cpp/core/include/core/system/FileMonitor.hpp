/*
 * FileMonitor.hpp
 *
 * Copyright (C) 2009-11 by RStudio, Inc.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 */

#ifndef CORE_SYSTEM_FILE_MONITOR_HPP
#define CORE_SYSTEM_FILE_MONITOR_HPP

#include <string>
#include <set>
#include <vector>

#include <boost/function.hpp>

#include <core/FilePath.hpp>
#include <core/collection/Tree.hpp>

#include <core/system/System.hpp>
#include <core/system/FileChangeEvent.hpp>

namespace core {   
namespace system {
namespace file_monitor {

// initialize the file monitoring service (creates a background thread
// which performs the monitoring)
void initialize();

// stop the file monitoring service (automatically unregisters all
// active file monitoring handles)
void stop();


// opaque handle to a registration (used to unregister). the id field
// is included so that handles have additional uniqueness beyond the
// value of the pData pointer (which could be duplicated accross monitors
// depending upon the order of allocations/deallocations)
struct Handle
{
   Handle()
      : pData(NULL)
   {
   }

   explicit Handle(void* pData)
      : id(core::system::generateUuid()),
        pData(pData)
   {
   }

   bool empty() const { return id.empty(); }

   bool operator==(const Handle& other) const
   {
      return id == other.id &&
             pData == other.pData;
   }

   std::string id;
   void* pData;
};

struct Callbacks
{
   // callback which occurs after a successful registration (includes an initial
   // listing of all of the files in the directory)
   boost::function<void(Handle, const tree<FileInfo>&)> onRegistered;

   // callback which occurs if a registration error occurs
   boost::function<void(const core::Error&)> onRegistrationError;

   // callback which occurs if an error occurs during monitoring (the
   // monitor is automatically unregistered if a monitoring error occurs)
   boost::function<void(const core::Error&)> onMonitoringError;

   // callback which occurs when files change
   boost::function<void(const std::vector<FileChangeEvent>&)> onFilesChanged;
};

// register a new file monitor. the result of this call will be an
// aynchronous call to either onRegistered or onRegistrationError. onRegistered
// will provide an opaque Handle which can used for a subsequent call
// to unregisterMonitor. if you want to bind a c++ object to the lifetime
// of this file monitor simply create a shared_ptr and bind its members
// to the file monitor callbacks.
void registerMonitor(const core::FilePath& filePath,
                     bool recursive,
                     const boost::function<bool(const FileInfo&)>& filter,
                     const Callbacks& callbacks);

// unregister a file monitor. note that file monitors can be automatically
// unregistered in the case of errors or a call to global file_monitor::stop,
// as a result multiple calls to unregisterMonitor are permitted (and no-op
// if the handle has already been unregistered)
void unregisterMonitor(Handle handle);


// check for changes (will cause onRegistered, onRegistrationError,
// onMonitoringError, and onFilesChanged to occur on the same thread that calls
// checkForChanges)
void checkForChanges();



// convenience functions for creating filters that are useful in
// file monitoring scenarios

// filter out any directory (and its children) with the specified name
// (no matter where it is located within the tree). useful for directories
// like .git, .svn, .RProj.user, etc.
boost::function<bool(const FileInfo&)> excludeDirectoryFilter(
                                                     const std::string& name);

// aggregate version of above
boost::function<bool(const FileInfo&)> excludeDirectoriesFilter(
                                    const std::vector<std::string>& names);

// exclude hidden files
boost::function<bool(const FileInfo&)> excludeHiddenFilter();


} // namespace file_monitor
} // namespace system
} // namespace core 

#endif // CORE_SYSTEM_FILE_MONITOR_HPP


