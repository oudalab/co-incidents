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
                ":event_cc_proto",
	],
)
cc_binary(
    name = "read",
    srcs = ["readEvents.cpp"],
        deps = [
                ":all-files",
                ":event_cc_proto",
        ],
)
# Convention:
# A cc_proto_library that wraps a proto_library named foo_proto
# should be called foo_cc_proto.
cc_proto_library(
    name = "event_cc_proto",
    deps=["event_proto"],
)
# Conventions:
# 1. One proto_library rule per .proto file.
# 2. A file named foo.proto will be in a rule named foo_proto.
proto_library(
  name = "event_proto",
    srcs = ["event.proto"],
   deps = [
        #":address_proto",
        # Well known protos should be included as deps in the
        # proto_library rules of the source files importing them.
        # A list of all @com_google_protobuf well known protos can
        # seen with:
        # `bazel query 'kind(proto_library, @com_google_protobuf//:all)'`
        "@com_google_protobuf//:any_proto",
    ],
)

cc_binary(
    name = "cross-year",
    srcs = ["cross-year.cpp"],
    deps = [
        ":all-files",
        ":event_cc_proto"
    ],
)
