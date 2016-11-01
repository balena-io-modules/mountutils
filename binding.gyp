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
            "src/functions_linux.cpp"
          ],
        } ],

        [ 'OS=="win"', { 
          "sources": [
            "src/mountutils.cpp",
            "src/functions_windows.cpp"
          ],
        } ],

        [ 'OS=="mac"', {
          "sources": [
            "src/mountutils.cpp",
            "src/functions_darwin.cpp"
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
