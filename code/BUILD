cc_library(
    name = "all-files",
    hdrs = glob([
        "*.h",
        "*.hpp",
    ]),
    copts = [
        "-march=native",
        "-mtune=native",
        "-O3",
        "-Wall",
    ],
    linkopts = [
        "-pthread",
        "-lsqlite3",
    ],
)

cc_binary(
    name = "co_yan",
    srcs = ["co_yan.cpp"],
	deps = [
		":all-files",
	],
)
