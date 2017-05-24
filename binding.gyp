{
  "targets": [
    {
      "target_name": "MountUtils",
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "sources": [
        "src/mountutils.cpp"
      ],
      'conditions': [

        [ 'OS=="linux"', {
          "sources": [
            "src/linux/functions.cpp"
          ],
        } ],

        [ 'OS=="win"', {
          "sources": [
            "src/windows/functions.cpp"
          ],
          "libraries": [
            "-luser32.lib",
            "-lsetupapi.lib",
          ],
        } ],

        [ 'OS=="mac"', {
          "sources": [
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
