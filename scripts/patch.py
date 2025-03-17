#######################################################
# The script to patch prompt_toolkit issue
#######################################################
import site

site_packages_paths = site.getsitepackages()

for path in site_packages_paths:
    file_path = path + '/prompt_toolkit/styles/from_dict.py'
    print(file_path)

    # old_sentence
    old_sentence = 'from collections import Mapping'
    # new_sentence
    new_sentence = 'from collections.abc import Mapping'

    # open and read
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()

    # replace
    content = content.replace(old_sentence, new_sentence)

    # write back
    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(content)