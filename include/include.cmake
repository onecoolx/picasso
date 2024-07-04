# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

file(GLOB_RECURSE PICASSO_HEADERS ${PROJECT_ROOT}/include/*.h)

install(FILES ${PICASSO_HEADERS} DESTINATION include/picasso)


