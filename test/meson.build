test_sources = [
  'test.cc',
]

inc = include_directories('.', '../lib4color')

test_app = executable('test_app',
                      test_sources,
                      include_directories: inc,
                      link_with: four_color_lib)

test('4-color test', test_app)
