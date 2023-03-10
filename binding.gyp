{
    "variables": {
        "cflags_cc": [
            "-Wall",
            # "-Werror",
            "-O3",
            "-fexceptions",  # Boost on Linux wants this
            "-frtti"         # And this too.
        ],
        "include_dirs": [
            "./node_modules/nan",
            "<!(echo ${BOOST_ROOT})"
        ],
        # "libraries": [
        #     "<!(echo ${BOOST_LIBRARYDIR}/libboost_filesystem.dylib)"
        # ],
        "OTHER_CFLAGS": [  # for Mac builds
            "-Wno-unused-local-typedefs"
        ]
    },
    "conditions": [
        [
            "OS=='win'", {
                "variables": {
                    "include_dirs": [
                        "<!(echo %BOOST_ROOT%)"
                    ]
                }
            }
        ],
        [
            "OS=='mac'", {
                "variables": {
                    "include_dirs": [
                        "<!(echo ${BOOST_ROOT})"
                    ],
                    "libraries": [
                        # "<!(echo $(dirname ${BOOST_ROOT})/lib/libboost_filesystem.dylib)"
                        "<!(echo ${BOOST_LIBRARYDIR})/libboost_filesystem.dylib"
                        # "/usr/local/opt/boost_1_81_0"
                    ]
                }
            }
        ],
        [
            "OS=='linux'", {
                "variables": {
                    "include_dirs": [
                        "<!(echo ${BOOST_ROOT})"
                    ],
                    "libraries": [
                        "-lrt",
                        # "<!(echo $(dirname ${BOOST_ROOT})/lib/libboost_filesystem.so)"
                        "<!(echo $(dirname ${BOOST_LIBRARYDIR})/libboost_filesystem.so)"
                    ]
                }
            }
        ],

    ],
    "targets": [
        {
            "target_name": "<(module_name)",
            "sources": ["shared-file-view.cc"],
            "cflags_cc": ["<@(cflags_cc)"],
            "include_dirs": ["<@(include_dirs)"],
            "libraries": ["<@(libraries)"],
            "xcode_settings": {
                "MACOSX_DEPLOYMENT_TARGET": "12.0",
                "OTHER_CFLAGS": [
                    "<@(OTHER_CFLAGS)",
                    "-stdlib=libc++"
                ],
                "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                "GCC_ENABLE_CPP_RTTI": "-frtti"
            },
            "msvs_settings": {
                "VCLinkerTool": {
                    "AdditionalLibraryDirectories": [
                        "<!(echo %BOOST_ROOT%/stage/lib)"
                    ]
                }
            }
        },
        {
            "target_name": "action_after_build",
            "type": "none",
            "dependencies": ["<(module_name)"],
            "copies": [
                {
                    "files": ["<(PRODUCT_DIR)/<(module_name).node"],
                    "destination": "<(module_path)"
                }
            ]
        }
    ]
}
