import datetime
import time

import ulid

test_count = 1000000

if __name__ == '__main__':

    ul = str(ulid.new())
    dt = datetime.datetime.now()
    ts = datetime.datetime.now().timestamp()

    tests = [('create str: ',  'str(ulid.new())'),
             ('create str with datetime: ',  'str(ulid.from_timestamp(dt))'),
             ('create str with timestamp: ',  'str(ulid.from_timestamp(ts))'),
             ('decode datetime from str: ',  'ulid.from_str(ul).timestamp().datetime'),
             ('decode timestamp from str: ',  'ulid.from_str(ul).timestamp().int')]

    for title, func in tests:
        code = f'''
res = []
for _ in range(10):
    start = time.time()
    for _ in range(test_count):
        _ = {func}
    res.append(time.time() - start)
print('{title}', test_count, ': ', res)
    '''
        eval(compile(code, '<string>', 'exec'))
