from typing import Optional, Union
import datetime


def ulid(timestamp: Optional[Union[datetime.datetime, float]] = None) -> str:
    """
    Generate a ULID.

    :param timestamp: The timestamp to use. Defaults to the current time.
    :return: A ULID.
    """
    pass


def decode_timestamp(ulid: str) -> float:
    """
    Return timestamp of ULID in `float`.

    :param ulid: A ULID.
    :return: timestamp
    """
    pass


def decode_datetime(ulid: str) -> float:
    """
    Return timestamp of ULID in `datetime.datetime`.
    Timestamp can cause overflow, so if it's too large, please use `decode_timestamp` instead.

    :param ulid: A ULID.
    :return: timestamp in `datetime.datetime`
    """
    pass
