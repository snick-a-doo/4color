four_color_sources = [
  'main.cc',
]

int = include_directories('../lib4color')

four_color_app = executable('4color',
                            four_color_sources,
                            include_directories: inc,
                            dependencies: gtkmm_dep,
                            link_with: [four_color_lib])
