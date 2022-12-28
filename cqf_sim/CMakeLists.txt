add_library(CQF_SIM_L
	additional_methods.cpp
	filter.cpp
	test.cpp
)

add_executable(CQF_SIM
	additional_methods.cpp
	filter.cpp
	test.cpp
	main.cpp
)

target_include_directories(CQF_SIM
	PUBLIC
	"${PROJECT_SOURCE_DIR}/cqf_sim/headers/"
)

target_include_directories(CQF_SIM_L
	PUBLIC
	"${PROJECT_SOURCE_DIR}/cqf_sim/headers/"
)
