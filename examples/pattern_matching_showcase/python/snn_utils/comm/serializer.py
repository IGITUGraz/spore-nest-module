import ast
import logging

logger = logging.getLogger(__name__)

SERIALIZERS = {}


class Serializer(object):
    def _serialize(self, obj):
        raise NotImplementedError()

    def serialize(self, obj):
        try:
            return self._serialize(obj)
        except Exception as e:
            logging.exception("Serializing of object failed:\n{}".format(obj))
            raise e

    def _deserialize(self, msg):
        raise NotImplementedError()

    def deserialize(self, msg):
        try:
            return self._deserialize(msg)
        except Exception as e:
            logging.exception("Deserializing of message failed:\n{}".format(msg))
            raise e


class ReprSerializer(Serializer):
    def _serialize(self, obj):
        return repr(obj)

    def _deserialize(self, msg):
        return ast.literal_eval(msg)

SERIALIZERS['repr'] = ReprSerializer()

try:
    import ujson

    class UJsonSerializer(Serializer):
        def _serialize(self, obj):
            return ujson.encode(obj)

        def _deserialize(self, msg):
            return ujson.decode(msg)

    # WARNING: json will transform tuples to strings; this can lead to errors depending on the sent data structures
    SERIALIZERS['ujson'] = UJsonSerializer()

except ImportError:
    pass
