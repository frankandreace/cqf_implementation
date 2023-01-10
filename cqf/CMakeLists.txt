add_library(CQF_L
	additional_methods.cpp
	filter.cpp
	test.cpp
)

add_executable(CQF
	filter.cpp
	additional_methods.cpp
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
