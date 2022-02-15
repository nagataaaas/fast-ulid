import datetime
import unittest

import fast_ulid


def parse_ulid(ulid):
    # this should work
    ENCODING = "0123456789ABCDEFGHJKMNPQRSTVWXYZ"
    timestamp = 0
    for i in range(10):
        timestamp <<= 5
        timestamp += ENCODING.index(ulid[i])
    return datetime.datetime.fromtimestamp(timestamp * 0.001, datetime.timezone.utc)


def increment_ulid(ulid):
    # this should work
    ENCODING = "0123456789ABCDEFGHJKMNPQRSTVWXYZ"
    randomness = [0 for _ in range(16)]
    for i in range(16):
        randomness[i] = ENCODING.index(ulid[10 + i])
    randomness[-1] += 1
    for i in range(15, -1, -1):
        if randomness[i] >= 32 and i != 0:
            randomness[i] = 0
            randomness[i - 1] += 1
        elif randomness[i] >= 32 and i == 0:
            raise ValueError
        else:
            break
    return ulid[:10] + "".join(ENCODING[i] for i in randomness)


class TestFastULID(unittest.TestCase):

    def test_create_ulid_now(self):
        ulid = fast_ulid.ulid()
        self.assertIsInstance(ulid, str)
        self.assertEqual(len(ulid), 26)  # 10 of timestamp + 16 of randomness

    def test_create_ulid_with_time_and_decode(self):
        now = datetime.datetime.now(datetime.timezone.utc)
        now = now.replace(microsecond=0)
        ulid = fast_ulid.ulid(now)
        self.assertIsInstance(ulid, str)
        self.assertEqual(len(ulid), 26)  # 10 of timestamp + 16 of randomness

        self.assertEqual(parse_ulid(ulid), now)
        self.assertEqual(fast_ulid.decode_datetime(ulid), now)
        self.assertEqual(fast_ulid.decode_timestamp(ulid), now.timestamp())
        self.assertEqual(fast_ulid.decode_timestamp('7ZZZZZZZZZZZZZZZZZZZZZZZZZ'), 281474976710.655)

    def test_parse_Error(self):
        ulid = fast_ulid.ulid()
        self.assertRaises(ValueError, fast_ulid.decode_timestamp, 'a' + ulid[1:])  # invalid character
        self.assertRaises(ValueError, fast_ulid.decode_timestamp, ulid[:-2])  # invalid length
        self.assertRaises(ValueError, fast_ulid.decode_timestamp, ulid[:-2])  # invalid length

        self.assertRaises(OverflowError, fast_ulid.decode_datetime,
                          '7ZZZZZZZZZZZZZZZZZZZZZZZZZ')  # Python can't handle this far future with fromtimestamp
        self.assertRaises(OverflowError, fast_ulid.decode_datetime,
                          '8000000000ZZZZZZZZZZZZZZZZ')  # This far future should be rejected to prevent overflow
        self.assertRaises(OverflowError, fast_ulid.decode_timestamp,
                          '8000000000ZZZZZZZZZZZZZZZZ')  # This far future should be rejected to prevent overflow

    def test_same_milli_second_increment(self):
        now = datetime.datetime.now(datetime.timezone.utc)
        ulids = [fast_ulid.ulid(now) for _ in range(1000)]
        for i in range(len(ulids) - 1):
            self.assertEqual(ulids[i + 1], increment_ulid(ulids[i]))


if __name__ == '__main__':
    unittest.main()
