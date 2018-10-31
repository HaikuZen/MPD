input_api = static_library(
  'input_api',
  'Error.cxx',
  'InputPlugin.cxx',
  'InputStream.cxx',
  'ThreadInputStream.cxx',
  'AsyncInputStream.cxx',
  'ProxyInputStream.cxx',
  include_directories: inc,
)

input_api_dep = declare_dependency(
  link_with: input_api,
  dependencies: [
    event_dep,
  ],
)

subdir('plugins')

input_glue = static_library(
  'input_glue',
  'Init.cxx',
  'Registry.cxx',
  'Open.cxx',
  'LocalOpen.cxx',
  'ScanTags.cxx',
  'Reader.cxx',
  'TextInputStream.cxx',
  'ProxyInputStream.cxx',
  'RewindInputStream.cxx',
  'BufferedInputStream.cxx',
  'MaybeBufferedInputStream.cxx',
  include_directories: inc,
)

input_glue_dep = declare_dependency(
  link_with: input_glue,
  dependencies: [
    input_plugins_dep,
    fs_dep,
    config_dep,
    tag_dep,
  ],
)