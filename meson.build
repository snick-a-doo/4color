# In the project directory, execute 'meson build/' once
# After that: 'ninja test -C build/'
# meson build/ --reconfigure --buildtype=debug
# meson build/ --reconfigure --buildtype=debugoptimized
# meson build/ --reconfigure --buildtype=release

project('4color', 'cpp',
        default_options: ['cpp_std=c++2a', 'warning_level=3'],
        version: '0.0.1',
        license: 'GPL3')

gtkmm_dep = dependency('gtkmm-3.0')

subdir('lib4color')
subdir('test')
subdir('app')
