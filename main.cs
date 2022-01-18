/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 TheVice
 *
 */

using Snappy;
using System.IO;

namespace SnappyRunner
{
    public static class Program
    {
        static void Compress(string input_path, string output_path)
        {
            using (var output = File.Create(output_path))
            {
                using (var input = File.OpenRead(input_path))
                {
                    Core.Compress(input, output);
                }
            }
        }

        static void GetUncompressedLength(string input_path, string output_path)
        {
            using (var output = File.Create(output_path))
            {
                uint result;
                bool return_;

                using (var input = File.OpenRead(input_path))
                {
                    return_ = Core.GetUncompressedLength(input, out result);
                }

                var s = string.Format("{0}\n{1}", return_ ? 1 : 0, result);
                var b = System.Text.Encoding.ASCII.GetBytes(s);
                output.Write(b, 0, b.Length);
            }
        }

        static void IsValidCompressedBuffer(string input_path, string output_path)
        {
            using (var output = File.Create(output_path))
            {
                bool return_;

                using (var input = File.OpenRead(input_path))
                {
                    return_ = Core.IsValidCompressed(input);
                }

                var s = string.Format("{0}", return_ ? 1 : 0);
                var b = System.Text.Encoding.ASCII.GetBytes(s);
                output.Write(b, 0, b.Length);
            }
        }

        static void Pack(string input_path, string output_path)
        {
            using (var output = File.Create(output_path))
            {
                using (var input = File.OpenRead(input_path))
                {
                    Core.Pack(input, output);
                }
            }
        }

        static void Uncompress(string input_path, string output_path)
        {
            using (var output = File.Create(output_path))
            {
                using (var input = File.OpenRead(input_path))
                {
                    byte[] result;

                    try
                    {
                        result = Core.Uncompress(input);
                    }
                    catch
                    {
                        result = null;
                    }

                    if (null != result)
                    {
                        output.Write(result, 0, result.Length);
                    }
                }
            }
        }

        static void UncompressAsMuchAsPossible(string input_path, string output_path)
        {
            using (var output = File.Create(output_path))
            {
                using (var input = File.OpenRead(input_path))
                {
                    Core.UncompressAsMuchAsPossible(input, output);
                }
            }
        }

        static readonly int EXIT_SUCCESS = 0;
        static readonly int EXIT_FAILURE = 1;

        public static int Main(string[] args)
        {
            if (args.Length != 3)
            {
                return EXIT_FAILURE;
            }

            switch (args[0])
            {
                case "Compress":
                    Compress(args[1], args[2]);
                    break;

                case "GetUncompressedLength":
                    GetUncompressedLength(args[1], args[2]);
                    break;

                case "IsValidCompressedBuffer":
                    IsValidCompressedBuffer(args[1], args[2]);
                    break;

                case "Pack":
                    Pack(args[1], args[2]);
                    break;

                case "Uncompress":
                    Uncompress(args[1], args[2]);
                    break;

                case "UncompressAsMuchAsPossible":
                    UncompressAsMuchAsPossible(args[1], args[2]);
                    break;

                default:
                    return EXIT_FAILURE;
            }

            return EXIT_SUCCESS;
        }
    }
}
