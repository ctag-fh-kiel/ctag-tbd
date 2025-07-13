from unittest import TestCase

from tbd_core.api.tbd_client import Packet, Header, PacketType, dump_packet, parse_packet

class TestHeader(TestCase):
    @staticmethod
    def short_encoded_packet():
        header = bytes([0xf0, 0x0, 0x3e, 0x79, 0x0, 0x62, 0x0, 0xe0, 0xd6, 0xe7])
        tail = bytes([0xff])
        return header + TestHeader.short_payload() + tail

    @staticmethod
    def long_encoded_packet():
        header = bytes([0xf0, 0x1, 0xfe, 0x0, 0x0, 0x22, 0x1, 0xfe, 0x0, 0xe7])
        tail = bytes([0xff])
        return header + TestHeader.long_payload() + tail

    @staticmethod
    def short_payload():
        return b'some medium sized payload that can be transferred, this will not hit the upper payload length byte'

    @staticmethod
    def long_payload():
        return b'some medium sized payload that can be transferred, this will hit the upper payload length byte...' \
            b'so we have to ramble on and on until we hit that magical 256 byte mark just like one of those really' \
            b'boring books we all had to read at school, when we would rather have been playing video games'

    def test_is_serializable(self):
        payload=b'serialize me'
        packet = Packet(
            type=PacketType.TYPE_EVENT,
            handler=3,
            id=7,
            payload_length=len(payload),
            crc=0,
            payload=payload,
        )
        encoded = dump_packet(packet)
        decoded = parse_packet(encoded)

        self.assertEqual(decoded.type, packet.type)
        self.assertEqual(decoded.handler, packet.handler)
        self.assertEqual(decoded.id, packet.id)
        self.assertEqual(decoded.payload_length, packet.payload_length)
        self.assertEqual(decoded.crc, packet.crc)
        self.assertEqual(decoded.payload, packet.payload)

    def test_allows_payloads_up_to_two_bytes_of_addressing(self):
        payload = b'Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ' \
            b'ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo ' \
            b'dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit ' \
            b'amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt' \
            b'ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo ' \
            b'dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.'
        packet = Packet(
            type=PacketType.TYPE_RPC,
            handler=55323,
            id=43232,
            payload_length=len(payload),
            crc=0,
            payload=payload,
        )
        encoded = dump_packet(packet)
        decoded = parse_packet(encoded)

        self.assertEqual(decoded.type, packet.type)
        self.assertEqual(decoded.handler, packet.handler)
        self.assertEqual(decoded.id, packet.id)
        self.assertEqual(decoded.payload_length, packet.payload_length)
        self.assertEqual(decoded.crc, packet.crc)
        self.assertEqual(decoded.payload, packet.payload)

    def test_is_serialized_to_binary_format_spec(self):
        payload = self.short_payload()
        packet = Packet(
            type=PacketType.TYPE_RPC,
            handler=31038,
            id=55008,
            payload_length=len(payload),
            crc=0,
            payload=payload,
        )
        encoded = dump_packet(packet)
        self.assertEqual(encoded, self.short_encoded_packet())

        payload=self.long_payload()
        packet = Packet(
            type=PacketType.TYPE_RESPONSE,
            handler=254,
            id=254,
            payload_length=len(payload),
            crc=0,
            payload=payload,
        )

        encoded = dump_packet(packet)
        self.assertEqual(encoded, self.long_encoded_packet())

    def test_is_deserialized_to_binary_format_spec(self):
        decoded = parse_packet(self.short_encoded_packet())
        self.assertEqual(decoded.type, PacketType.TYPE_RPC)
        self.assertEqual(decoded.handler, 31038)
        self.assertEqual(decoded.id, 55008)
        self.assertEqual(decoded.payload_length, len(self.short_payload()))
        self.assertEqual(decoded.crc, 0)
        self.assertEqual(decoded.payload, self.short_payload())

        decoded = parse_packet(self.long_encoded_packet())
        self.assertEqual(decoded.type, PacketType.TYPE_RESPONSE)
        self.assertEqual(decoded.handler, 254)
        self.assertEqual(decoded.id, 254)
        self.assertEqual(decoded.payload_length, len(self.long_payload()))
        self.assertEqual(decoded.crc, 0)
        self.assertEqual(decoded.payload, self.long_payload())

    def test_print_short(self):
        payload=self.short_payload()
        packet = Packet(
            type=PacketType.TYPE_RPC,
            handler=31038,
            id=55008,
            payload_length=len(payload),
            crc=0,
            payload=payload,
        )

        encoded = dump_packet(packet)
        print('short', len(encoded), len(payload))
        print([hex(b) for b in encoded])

    def test_pr_long(self):
        payload=self.long_payload()
        packet = Packet(
            type=PacketType.TYPE_RESPONSE,
            handler=254,
            id=254,
            payload_length=len(payload),
            crc=0,
            payload=payload,
        )

        encoded = dump_packet(packet)
        print('long', len(encoded), len(payload))
        print([hex(b) for b in encoded])

