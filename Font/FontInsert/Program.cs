using System;
using System.IO;

namespace ConsoleApp1
{
    class Program
    {
        static void Main(string[] args)
        {
           byte[] ROM = File.ReadAllBytes(args[0]);
            byte[] f1 = File.ReadAllBytes(args[1]);
            byte[] f2 = File.ReadAllBytes(args[2]);


            f1.CopyTo(ROM, 0x00928000);
            f2.CopyTo(ROM, 0x00B88EA0);

            File.WriteAllBytes(args[0], ROM);


        }
    }
}
