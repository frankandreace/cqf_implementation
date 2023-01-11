add_library(CQF
	additional_methods.cpp
	filter.cpp
)

target_include_directories(CQF
	PUBLIC
	"${PROJECT_SOURCE_DIR}/cqf/headers/"
)
