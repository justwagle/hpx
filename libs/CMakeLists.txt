# Copyright (c) 2019 The STE||AR-Group
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# This file is auto generated. Please do not edit manually.

include(HPX_Message)

set(HPX_LIBS
  cache
  config
  preprocessor
  CACHE INTERNAL "" FORCE
)

hpx_info("Configuring modules:")

foreach(lib ${HPX_LIBS})
  add_subdirectory(${lib})
endforeach()

