{
  "targets": [
    {
      "cflags!": [
        "-fno-exceptions"
      ],
      "cflags_cc!": [
        "-fno-exceptions"
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include_dir\")"
      ],
      "target_name": "nodeeasyipc",
      "conditions": [
        [
          'OS=="win"',
          {
            "sources": [
              "src/windows/addon.cc",
              "src/windows/filemap.cc",
              "src/windows/mutex.cc",
              "src/windows/ipc.cc"
            ],
            "defines": [
              "WINDOWS_BUILD"
            ]
          }
        ],
        [
          'OS!="win"',
          {
            "defines": [
              "UNIX_BUILD"
            ]
          }
        ]
      ]
    }
  ]
}
