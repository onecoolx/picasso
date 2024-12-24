# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

file(GLOB_RECURSE PICASSO_HEADERS ${PROJECT_ROOT}/include/picasso.h)
install(FILES ${PICASSO_HEADERS} DESTINATION include/picasso)

file(GLOB_RECURSE PSX_IMAGES_HEADERS ${PROJECT_ROOT}/include/images/*.h)
install(FILES ${PSX_IMAGES_HEADERS} DESTINATION include/picasso/images)

