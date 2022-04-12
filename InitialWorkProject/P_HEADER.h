#pragma once
#ifndef P_HEADER_H
#define P_HEADER_H

#include <struct_mapping/struct_mapping.h>

#include <CoreToolkit/Logger.h>
#include <CoreToolkit/OSUtils.h>
#include <CoreToolkit/Util.h>
#include <CoreToolkit/File.h>
#include <CoreToolkit/FileUtils.h>

#include <WebToolkit/WebToolkit.h>

#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <iostream>


#if __cplusplus < 201703L && !defined(WIN32)
#error("C++ Standard need at least 17")
#endif

#endif