{
  "targets": [
    {
      "target_name": "MountUtils",
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      'conditions': [

        [ 'OS=="linux"', {
          "sources": [
            "src/mountutils.cpp",
            "src/linux/functions.cpp"
          ],
        } ],

        [ 'OS=="win"', {
          "sources": [
            "src/mountutils.cpp",
            "src/windows/functions.cpp"
          ],
          "libraries": [
            "-luser32.lib",
            "-lsetupapi.lib",
          ],
        } ],

        [ 'OS=="mac"', {
          "sources": [
            "src/mountutils.cpp",
            "src/darwin/functions.cpp"
          ],
          "link_settings": {
            "libraries": [
              "DiskArbitration.framework",
              "Foundation.framework",
            ],
          },
        } ],

      ],
    }
  ],
}
