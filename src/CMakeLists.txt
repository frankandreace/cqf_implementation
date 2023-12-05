#add_library(RSQF
#	additional_methods.cpp
#	rsqf.cpp
#	abstract_bqf.cpp
#	bqf_ec.cpp
#	bqf_oom.cpp
#	cqf.cpp
#)

add_executable(mytest 
	main.cpp 
	additional_methods.cpp
	rsqf.cpp
	cqf.cpp
)

#target_include_directories(RSQF
#	PUBLIC
#	"${PROJECT_SOURCE_DIR}/src/headers/"
#)

target_include_directories(mytest
	PUBLIC
	"${PROJECT_SOURCE_DIR}/src/headers/"
)

#set_property(TARGET RSQF PROPERTY CXX_STANDARD 17)
set_property(TARGET mytest PROPERTY CXX_STANDARD 17)
