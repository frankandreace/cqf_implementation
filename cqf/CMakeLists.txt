add_library(CQF_donL
	additional_methods.cpp
	filter.cpp
	test.cpp
)

add_executable(CQF
	additional_methods.cpp
	filter.cpp
	test.cpp
	main.cpp
)

target_include_directories(CQF
	PUBLIC
	"${PROJECT_SOURCE_DIR}/cqf/headers/"
)

target_include_directories(CQF_L
	PUBLIC
	"${PROJECT_SOURCE_DIR}/cqf/headers/"
)
