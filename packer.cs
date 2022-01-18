/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

using System;
using System.IO;

namespace Snappy
{
    public static class Packer
    {
        static void Pack(string[] inputs, string output)
        {
            long totalLength = 0;

            foreach (var input in inputs)
            {
                using (var reader = File.OpenRead(input))
                {
                    totalLength += reader.Length;
                }
            }

            using (var writer = File.Create(output))
            {
                Varints.Write(Misc.UInt64ToBin((ulong)totalLength), writer);

                foreach (var input in inputs)
                {
                    using (var reader = File.OpenRead(input))
                    {
                        Literal.Write((uint)reader.Length, writer);
                        reader.CopyTo(writer);
                    }
                }
            }
        }

        static void UnPack(string input, int number_of_file, string output)
        {
            using (var reader = File.OpenRead(input))
            {
                var l = Varints.Read(reader);

                if (null == l)
                {
                    throw new Exception("Incorrect Varint data.");
                }

                int tag;
                int literalNumber = 0;

                while (-1 != (tag = reader.ReadByte()))
                {
                    var element_type = (byte)(tag & 0x3);

                    if (0 != element_type)
                    {
                        throw new Exception("Unexpected element type at the UnPack function.");
                    }

                    uint length;
                    Literal.Read((byte)tag, reader, out length);

                    if (number_of_file == literalNumber)
                    {
                        using (var writer = File.Create(output))
                        {
                            Misc.CopyTo(reader, length, writer);
                        }

                        return;
                    }

                    reader.Seek(length, SeekOrigin.Current);
                    ++literalNumber;
                }

                var message = string.Format("File number {0} could not be found at the current pack '{1}' with {2} literal(s).", number_of_file, input, literalNumber);
                throw new Exception(message);
            }
        }

        public static void Main(string[] args)
        {
            if (args.Length == 0 ||
                ("pack" != args[0]) &&
                ("unpack" != args[0]))
            {
                Console.WriteLine("pack|unpack parameters");
                return;
            }

            if ("pack" == args[0])
            {
                if (args.Length - 2 < 1)
                {
                    Console.WriteLine("pack input_1 input_2 ... input_n output");
                    return;
                }

                var inputs = new string[args.Length - 2];
                Array.Copy(args, 1, inputs, 0, args.Length - 2);
                Pack(inputs, args[args.Length - 1]);
            }
            else
            {
                if (4 != args.Length)
                {
                    Console.WriteLine("unpack input number_of_file output");
                    return;
                }

                var number_of_file = int.Parse(args[2]);
                UnPack(args[1], number_of_file, args[3]);
            }
        }
    }
}
