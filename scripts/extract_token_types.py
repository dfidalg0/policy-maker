import re

regex = re.compile(r'#ifndef YYTOKENTYPE(?:.|\n)+?#endif')

with open('target/parser.yy.hh') as f:
    content = f.read()

    text = regex.search(content).group(0)

    content = content.replace(text, '#include <token_types.hh>')


with open('target/parser.yy.hh', 'w') as f:
    f.write(content)

with open('target/token_types.hh', 'w') as f:
    f.write(text)
