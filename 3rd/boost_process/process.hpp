// 
// Boost.Process 
// ~~~~~~~~~~~~~ 
// 
// Copyright (c) 2006, 2007 Julio M. Merino Vidal 
// Copyright (c) 2008, 2009 Boris Schaeling 
// 
// Distributed under the Boost Software License, Version 1.0. (See accompanying 
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) 
// 

/** 
 * \file boost/process.hpp 
 * 
 * Convenience header that includes all other Boost.Process public header 
 * files. It is important to note that those headers that are specific to 
 * a given platform are only included if the library is being used in that 
 * same platform. 
 */ 

#ifndef BOOST_PROCESS_HPP 
#define BOOST_PROCESS_HPP 

#include <boost_process/process/config.hpp> 

#if defined(BOOST_POSIX_API) 
#  include <boost_process/process/posix_child.hpp> 
#  include <boost_process/process/posix_context.hpp> 
#  include <boost_process/process/posix_operations.hpp> 
#  include <boost_process/process/posix_status.hpp> 
#elif defined(BOOST_WINDOWS_API) 
#  include <boost_process/process/win32_child.hpp> 
#  include <boost_process/process/win32_context.hpp> 
#  include <boost_process/process/win32_operations.hpp> 
#else 
#  error "Unsupported platform." 
#endif 

#include <boost_process/process/child.hpp> 
#include <boost_process/process/context.hpp> 
#include <boost_process/process/environment.hpp> 
#include <boost_process/process/operations.hpp> 
#include <boost_process/process/pistream.hpp> 
#include <boost_process/process/postream.hpp> 
#include <boost_process/process/process.hpp> 
#include <boost_process/process/self.hpp> 
#include <boost_process/process/status.hpp> 
#include <boost_process/process/stream_behavior.hpp> 

#endif 
