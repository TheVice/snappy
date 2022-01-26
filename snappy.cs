/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

using System;
using System.IO;
using System.Collections.Generic;

namespace Snappy
{
    static class Misc
    {
        internal static uint CopyTo(Stream input, uint count, Stream output)
        {
            var buffer = new byte[4096];

            for (uint i = 0; i < count; i += (uint)buffer.Length)
            {
                var length = count - i;

                if (buffer.Length < length)
                {
                    length = (uint)buffer.Length;
                }

                if (length != input.Read(buffer, 0, (int)length))
                {
                    return i;
                }

                output.Write(buffer, 0, (int)length);
            }

            return count;
        }
    }

    public static class Varints
    {
        static readonly byte[] data = new byte[6];

        public static bool From(Stream input, out uint output)
        {
            output = 0;
            int byte_data;
            uint multi = 1;

            while (-1 != (byte_data = input.ReadByte()))
            {
                if (byte_data < 128)
                {
                    while (true)
                    {
                        output += 0 < (byte_data & 0x1) ? multi : 0;
                        byte_data >>= 1;

                        if (0 == byte_data)
                        {
                            break;
                        }

                        if (2147483648 == multi)
                        {
                            return false;
                        }

                        multi <<= 1;
                    }

                    byte_data = short.MaxValue;
                    break;
                }

                byte count = 7;
                byte_data &= 127;

                while (true)
                {
                    output += 0 < (byte_data & 0x1) ? multi : 0;
                    byte_data >>= 1;

                    if (0 == byte_data)
                    {
                        break;
                    }

                    if (2147483648 == multi)
                    {
                        return false;
                    }

                    multi <<= 1;
                    --count;
                }

                while (0 < count)
                {
                    if (2147483648 == multi)
                    {
                        return false;
                    }

                    multi <<= 1;
                    --count;
                }
            }

            if (short.MaxValue != byte_data)
            {
                return false;
            }

            return true;
        }

        public static void To(uint input, Stream output)
        {
            if (input < 128)
            {
                output.WriteByte((byte)input);
                return;
            }

            byte i = 0, multi = 1;
            Array.Clear(data, 0, data.Length);

            while (0 < input)
            {
                if (128 == multi)
                {
                    data[i++] += multi;
                    multi = 1;
                }

                data[i] += (byte)(0 < (input & 0x1) ? multi : 0);
                input >>= 1;
                multi <<= 1;
            }

            output.Write(data, 0, i + 1);
        }
    }

    public static class Literal
    {
        static readonly byte[] data = new byte[4];

        public static bool Read(byte tag, Stream input, out uint length)
        {
            length = 0;
            tag >>= 2;
            ++tag;

            if (60 < tag)
            {
                tag -= 60;

                if (data.Length < tag)
                {
                    return false;
                }

                if (tag != input.Read(data, 0, tag))
                {
                    return false;
                }

                Array.Clear(data, tag, data.Length - tag);
                length = BitConverter.ToUInt32(data, 0) + 1;
            }
            else
            {
                length = tag;
            }

            return true;
        }

        public static void Write(uint length, Stream output)
        {
            if (length < 1)
            {
                var message = string.Format(
                    "count can not be less than 1, instead it {0}.", length);
                throw new ArgumentOutOfRangeException(message);
            }

            --length;
            byte i = 0;

            if (59 < length)
            {
                Array.Copy(BitConverter.GetBytes(length), data, data.Length);

                if (0 < data[3])
                {
                    length = 63;
                }
                else if (0 < data[2])
                {
                    length = 62;
                }
                else if (0 < data[1])
                {
                    length = 61;
                }
                else
                {
                    length = 60;
                }

                i = (byte)(length - 59);
            }

            length <<= 2;
            output.WriteByte((byte)length);

            if (0 < i)
            {
                output.Write(data, 0, i);
            }
        }
    }

    static class RunLength
    {
        static readonly byte[] data = new byte[byte.MaxValue + 1];

        internal static byte[] Decoding(byte[] readedData, uint readedDataLength, uint offset, byte length)
        {
            if (readedDataLength < offset ||
                0 == offset)
            {
                return null;
            }

            var sourceIndex = readedDataLength - offset;

            if (readedDataLength < sourceIndex + length)
            {
                byte index = 0;
                var len = (byte)(readedDataLength - sourceIndex);

                while (index + len < length)
                {
                    Array.Copy(readedData, (int)sourceIndex, data, index, len);
                    index += len;
                }

                len = (byte)(length - index);

                if (0 < len)
                {
                    Array.Copy(readedData, (int)sourceIndex, data, index, len);
                }
            }
            else
            {
                Array.Copy(readedData, (int)sourceIndex, data, 0, length);
            }

            return data;
        }

        internal static byte[] Decoding(Stream uncompressed, uint offset, byte length)
        {
            var originalPosition = uncompressed.Position;

            if (originalPosition < offset ||
                0 == offset)
            {
                return null;
            }

            var sourceIndex = originalPosition - offset;
            uncompressed.Seek(sourceIndex, SeekOrigin.Begin);

            if (originalPosition < sourceIndex + length)
            {
                byte index = 0;
                var len = (byte)(originalPosition - sourceIndex);

                while (index + len < length)
                {
                    uncompressed.Seek(sourceIndex, SeekOrigin.Begin);
                    uncompressed.Read(data, index, len);
                    index += len;
                }

                len = (byte)(length - index);

                if (0 < len)
                {
                    uncompressed.Seek(sourceIndex, SeekOrigin.Begin);
                    uncompressed.Read(data, index, len);
                }
            }
            else
            {
                uncompressed.Read(data, 0, length);
            }

            uncompressed.Seek(originalPosition, SeekOrigin.Begin);
            return data;
        }
    }

    public static class BackReference
    {
        static readonly byte[] data = new byte[4];

        public static bool Read1byteOffset(byte tag, Stream input, out ushort offset, out byte length)
        {
            var byte_data = input.ReadByte();

            if (-1 == byte_data)
            {
                length = 0;
                offset = 0;
                return false;
            }

            tag >>= 2;
            length = (byte)(tag & 7);
            length += 4;
            //
            data[0] = (byte)byte_data;
            data[1] = (byte)(tag >> 3);
            offset = BitConverter.ToUInt16(data, 0);
            //
            return true;
        }

        public static void Write1byteOffset(ushort offset, byte length, Stream output)
        {
            if (2047 < offset || 11 < length || length < 4)
            {
                var message = string.Format(
                    "offset should be less than 2048, offset = {0}\n" +
                    "and length should be in range from 4 to the 11, length = {1}.", offset, (int)length);
                throw new ArgumentOutOfRangeException(message);
            }

            Array.Copy(BitConverter.GetBytes(offset), data, 2);
            //
            length -= 4;
            length &= 7;
            //
            length <<= 2;
            length |= 1;
            //
            data[1] <<= 5;
            length |= data[1];
            //
            output.WriteByte(length);
            output.WriteByte(data[0]);
        }

        public static bool Read2bytesOffset(byte tag, Stream input, out ushort offset, out byte length)
        {
            if (2 != input.Read(data, 0, 2))
            {
                length = 0;
                offset = 0;
                return false;
            }

            length = (byte)((tag >> 2) + 1);
            offset = BitConverter.ToUInt16(data, 0);
            return true;
        }

        static void WriteLength(byte length, byte tag, Stream output)
        {
            if (64 < length || length < 1)
            {
                var message = string.Format(
                    "length should be in range from 1 to the 64 instead it {0}.", (int)length);
                throw new ArgumentOutOfRangeException(message);
            }

            length -= 1;
            length <<= 2;
            length |= tag;
            output.WriteByte(length);
        }

        public static void WriteOffset(ushort offset, byte length, Stream output)
        {
            WriteLength(length, 2, output);
            Array.Copy(BitConverter.GetBytes(offset), data, 2);
            output.Write(data, 0, 2);
        }

        public static bool Read4bytesOffset(byte tag, Stream input, out uint offset, out byte length)
        {
            if (4 != input.Read(data, 0, 4))
            {
                length = 0;
                offset = 0;
                return false;
            }

            length = (byte)((tag >> 2) + 1);
            offset = BitConverter.ToUInt32(data, 0);
            return true;
        }

        public static void WriteOffset(uint offset, byte length, Stream output)
        {
            WriteLength(length, 3, output);
            Array.Copy(BitConverter.GetBytes(offset), data, 4);
            output.Write(data, 0, 4);
        }
    }

    class StreamByteReader
    {
        int data;
        readonly Stream stream;

        internal StreamByteReader(Stream stream)
        {
            this.stream = stream;
        }

        internal byte this[long i]
        {
            get
            {
                stream.Seek(i, SeekOrigin.Begin);
                data = stream.ReadByte();

                if (-1 == data)
                {
                    throw new IndexOutOfRangeException();
                }

                return (byte)data;
            }
        }

        internal long Length { get { return stream.Length; } }

        internal void CopyTo(long index, uint count, Stream output)
        {
            stream.Seek(index, SeekOrigin.Begin);

            if (count != Misc.CopyTo(stream, count, output))
            {
                throw new IndexOutOfRangeException();
            }
        }
    }

    public static class Core
    {
        public static void Pack(byte[] input, Stream output)
        {
            Varints.To((uint)input.Length, output);
            Literal.Write((uint)input.Length, output);
            output.Write(input, 0, input.Length);
        }

        public static void Pack(Stream input, Stream output)
        {
            Varints.To((uint)input.Length, output);
            Literal.Write((uint)input.Length, output);
            input.CopyTo(output);
        }

        static KeyValuePair<uint, byte> GetCopy(StreamByteReader reader, uint position, IEnumerable<uint> dictionary)
        {
            byte length = 0;
            uint index = uint.MaxValue;
            var max_length = Math.Min(64, reader.Length - position);

            if (4 < max_length)
            {
                foreach (var item in dictionary)
                {
                    byte i = 1;

                    while (item + i <= position && reader[position + i] == reader[item + i])
                    {
                        ++i;

                        if (max_length == i)
                        {
                            break;
                        }
                    }

                    if (length < i && 4 < i)
                    {
                        length = i;
                        index = item;
                    }

                    if (max_length == length)
                    {
                        break;
                    }
                }
            }

            return new KeyValuePair<uint, byte>(index, length);
        }

        public static void Compress(Stream input, Stream output)
        {
            var reader = new StreamByteReader(input);
            Varints.To((uint)reader.Length, output);
            //
            var data = new byte[4096];
            var dictionary = new List<uint>[256];
            uint index = 0;

            for (uint position = 0; ;)
            {
                input.Seek(position, SeekOrigin.Begin);
                var count = input.Read(data, 0, data.Length);

                if (count < 1)
                {
                    break;
                }

                for (int i = 0; i < count; ++i, ++position)
                {
                    if (null == dictionary[data[i]])
                    {
                        dictionary[data[i]] = new List<uint>();
                    }
                    else
                    {
                        var copy = GetCopy(reader, position, dictionary[data[i]]);

                        if (512 < dictionary[data[i]].Count)
                        {
                            dictionary[data[i]].RemoveRange(0, 256);
                        }

                        if (0 < copy.Value)
                        {
                            if (index < position)
                            {
                                var length = position - index;
                                Literal.Write(length, output);
                                reader.CopyTo(index, length, output);
                                index = position;
                            }

                            var offset = index - copy.Key;

                            if (ushort.MaxValue < offset)
                            {
                                BackReference.WriteOffset(offset, copy.Value, output);
                            }
                            else
                            {
                                if (offset < 2048 && 3 < copy.Value && copy.Value < 12)
                                {
                                    BackReference.Write1byteOffset((ushort)offset, copy.Value, output);
                                }
                                else
                                {
                                    BackReference.WriteOffset((ushort)offset, copy.Value, output);
                                }
                            }

                            dictionary[data[i]].Add(position);
                            index += copy.Value;
                            position += copy.Value;
                            i += copy.Value;
                        }
                        else
                        {
                            dictionary[data[i]].Add(position);
                        }
                    }
                }
            }

            if (index < reader.Length)
            {
                var length = (uint)(reader.Length - index);
                Literal.Write(length, output);
                reader.CopyTo(index, length, output);
                index = (uint)reader.Length;
            }

            if (index != reader.Length)
            {
                throw new InvalidDataException();
            }
        }

        public static void Compress(byte[] input, Stream output)
        {
            using (var input_stream = new MemoryStream(input))
            {
                Compress(input_stream, output);
            }
        }

        public static void Compress(byte[] input, uint input_length, out byte[] compressed, out uint compressed_length)
        {
            using (var output = new MemoryStream())
            {
                using (var input_stream = new MemoryStream(input, 0, (int)input_length))
                {
                    Compress(input_stream, output);
                }

                compressed = output.ToArray();
                compressed_length = (uint)output.Length;
            }
        }

        public static bool GetUncompressedLength(Stream input, out uint result)
        {
            return Varints.From(input, out result);
        }

        public static bool IsValidCompressed(Stream input)
        {
            try
            {
                using (var output = new MemoryStream())
                {
                    Uncompress(input, output);
                }
            }
            catch (Exception)
            {
                return false;
            }

            return true;
        }

        public static bool IsValidCompressedBuffer(byte[] compressed, uint compressed_length)
        {
            using (var input_stream = new MemoryStream(compressed, 0, (int)compressed_length))
            {
                return IsValidCompressed(input_stream);
            }
        }

        public static void Uncompress(Stream compressed, Stream uncompressed)
        {
            if (!Varints.From(compressed, out uint uncompressed_length))
            {
                throw new Exception("Incorrect Varint data.");
            }

            uint length = 0;
            var current_length = uncompressed.Length;
            int tag;

            while (-1 != (tag = compressed.ReadByte()))
            {
                var element_type = (byte)(tag & 0x3);
                ushort data_offset = 0;
                uint data_offset_4 = 0;
                byte data_length = 0;

                switch (element_type)
                {
                    case 0:
                        if (!Literal.Read((byte)tag, compressed, out data_offset_4) || 0 == data_offset_4)
                        {
                            throw new Exception("Tag byte of literal is incorrect or stream reach the finish that is unexpected.");
                        }

                        break;
                    case 1:
                        if (!BackReference.Read1byteOffset((byte)tag, compressed, out data_offset, out data_length))
                        {
                            throw new EndOfStreamException();
                        }

                        break;
                    case 2:
                        if (!BackReference.Read2bytesOffset((byte)tag, compressed, out data_offset, out data_length))
                        {
                            throw new EndOfStreamException();
                        }

                        break;
                    case 3:
                        if (!BackReference.Read4bytesOffset((byte)tag, compressed, out data_offset_4, out data_length))
                        {
                            throw new EndOfStreamException();
                        }

                        break;
                }

                if (0 < element_type)
                {
                    if (3 != element_type)
                    {
                        data_offset_4 = data_offset;
                    }

                    var data = RunLength.Decoding(uncompressed, data_offset_4, data_length);

                    if (null == data)
                    {
                        var message = string.Format("Offset ({0}) can not be equal to the zero or be more than length ({1}).", data_offset_4, length);
                        throw new ArgumentOutOfRangeException(message);
                    }

                    uncompressed.Write(data, 0, data_length);
                    length += data_length;
                }
                else
                {
                    if (data_offset_4 != Misc.CopyTo(compressed, data_offset_4, uncompressed))
                    {
                        throw new EndOfStreamException();
                    }

                    length += data_offset_4;
                }
            }

            if (length != uncompressed_length ||
                length != uncompressed.Length - current_length ||
                -1 != compressed.ReadByte())
            {
                throw new InvalidDataException();
            }
        }

        public static uint UncompressAsMuchAsPossible(Stream compressed, Stream uncompressed)
        {
            if (!Varints.From(compressed, out _))
            {
                return 0;
            }

            uint length = 0;
            int tag;

            while (-1 != (tag = compressed.ReadByte()))
            {
                var element_type = (byte)(tag & 0x3);
                ushort data_offset = 0;
                uint data_offset_4 = 0;
                byte data_length = 0;

                switch (element_type)
                {
                    case 0:
                        if (!Literal.Read((byte)tag, compressed, out data_offset_4) || 0 == data_offset_4)
                        {
                            return length;
                        }

                        break;
                    case 1:
                        if (!BackReference.Read1byteOffset((byte)tag, compressed, out data_offset, out data_length))
                        {
                            return length;
                        }

                        break;
                    case 2:
                        if (!BackReference.Read2bytesOffset((byte)tag, compressed, out data_offset, out data_length))
                        {
                            return length;
                        }

                        break;
                    case 3:
                        if (!BackReference.Read4bytesOffset((byte)tag, compressed, out data_offset_4, out data_length))
                        {
                            return length;
                        }

                        break;
                }

                if (0 < element_type)
                {
                    if (3 != element_type)
                    {
                        data_offset_4 = data_offset;
                    }

                    var data = RunLength.Decoding(uncompressed, data_offset_4, data_length);

                    if (null == data)
                    {
                        return length;
                    }

                    uncompressed.Write(data, 0, data_length);
                    length += data_length;
                }
                else
                {
                    if (data_offset_4 != Misc.CopyTo(compressed, data_offset_4, uncompressed))
                    {
                        return length;
                    }

                    length += data_offset_4;
                }
            }

            return length;
        }
    }
}
