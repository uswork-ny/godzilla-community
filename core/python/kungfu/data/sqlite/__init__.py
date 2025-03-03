'''
This is source code modified under the Apache License 2.0.
Original Author: Keren Dong
Modifier: kx@godzilla.dev
Modification date: March 3, 2025
'''
import pyyjj
import json
from pathlib import Path
from contextlib import contextmanager
from sqlalchemy import inspect, types, TypeDecorator
import kungfu.wingchun.constants as wc_constants

def make_url(location, filename):
    if filename == "accounts" or filename == "holidays" or filename == "ledger":
        db_file = location.locator.layout_file(location, pyyjj.layout.SQLITE, filename)
    else:
        path = Path(f"/dev/shm/strategies/{str(location.uid)}")
        path.mkdir(parents=True, exist_ok=True)
        db_file = f"/dev/shm/strategies/{str(location.uid)}/{filename}.db"
    return 'sqlite:///{}'.format(db_file)


def object_as_dict(obj):
    return {c.key: getattr(obj, c.key)
            for c in inspect(obj).mapper.column_attrs}

@contextmanager
def session_scope(session_factory):
    """Provide a transactional scope around a series of operations."""
    session = session_factory()
    try:
        yield session
        session.commit()
    except:
        session.rollback()
        raise
    finally:
        session.close()

class Json(TypeDecorator):
    @property
    def python_type(self):
        return object

    impl = types.String

    def process_bind_param(self, value, dialect):
        return json.dumps(value)

    def process_literal_param(self, value, dialect):
        return value

    def process_result_value(self, value, dialect):
        try:
            return json.loads(value)
        except (ValueError, TypeError):
            return None

class EnumTypeDecorator(TypeDecorator):
    impl = types.Integer
    def __init__(self, enum_type):
        TypeDecorator.__init__(self)
        self.enum_type = enum_type

    def coerce_compared_value(self, op, value):
        return self.impl.coerce_compared_value(op, value)

    def process_bind_param(self, value, dialect):
        return int(value)

    def process_literal_param(self, value, dialect):
        return value

    def process_result_value(self, value, dialect):
        try:
            return self.enum_type(value)
        except (ValueError, TypeError):
            return None

class VolumeCondition(EnumTypeDecorator):
    def __init__(self):
        super(VolumeCondition, self).__init__(wc_constants.VolumeCondition)

class TimeCondition(EnumTypeDecorator):
    def __init__(self):
        super(TimeCondition, self).__init__(wc_constants.TimeCondition)

class OrderStatus(EnumTypeDecorator):
    def __init__(self):
        super(OrderStatus, self).__init__(wc_constants.OrderStatus)

class InstrumentType(EnumTypeDecorator):
    cache_ok = True

    def __init__(self):
        super(InstrumentType, self).__init__(wc_constants.InstrumentType)

class Side(EnumTypeDecorator):
    def __init__(self):
        super(Side, self).__init__(wc_constants.Side)

class Offset(EnumTypeDecorator):
    def __init__(self):
        super(Offset, self).__init__(wc_constants.Offset)

class Direction(EnumTypeDecorator):
    def __init__(self):
        super(Direction, self).__init__(wc_constants.Direction)

class OrderType(EnumTypeDecorator):
    def __init__(self):
        super(OrderType, self).__init__(wc_constants.OrderType)

class LedgerCategory(EnumTypeDecorator):
    def __init__(self):
        super(LedgerCategory, self).__init__(wc_constants.LedgerCategory)

class UINT64(TypeDecorator):
    impl = types.String
    cache_ok = True

    def coerce_compared_value(self, op, value):
        return self.impl.coerce_compared_value(op, value)

    def process_bind_param(self, value, dialect):
        return str(value)

    def process_literal_param(self, value, dialect):
        return value

    def process_result_value(self, value, dialect):
        try:
            return int(value)
        except (ValueError, TypeError):
            return None