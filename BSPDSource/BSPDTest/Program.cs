using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace BSPDTest
{
    class Program
    {
        struct MCTx
        {
           public int num;
           public int x;
           public int y;
           public IntPtr path;
        }

        public delegate void BSPDLogCallback(string log); 
        struct BSPDContext
        {
            public IntPtr   pCoder;
            public IntPtr   inputPath;
            public IntPtr   optiongs;
            public int codedWidth;
            public int codedHeight;
            public int width;
            public int height;
            public int ysize;
            public int opcode;
            public int timeStamp;
            public int vFps;
            public int vDuration;
        }

        [DllImport("BSPD")]
        private static extern long dlltest();

        [DllImport("BSPD")]
        private static extern long getctx(ref MCTx cTx);

        [DllImport("BSPD")]
        private static extern IntPtr createctx();

        [DllImport("BSPD")]
        private static extern IntPtr BSPDCreateCtx();

        [DllImport("BSPD")]
        private static extern int BSPDSetLogCallback(IntPtr ctx,BSPDLogCallback callback);

        [DllImport("BSPD")]
        private static extern int BSPDClose(IntPtr ctx);

        static void LogCallbackFunc(string log)
        {
            Console.Write(log);
        }

        static void Main(string[] args)
        {

            IntPtr bspdctx = BSPDCreateCtx();

            BSPDSetLogCallback(bspdctx, LogCallbackFunc);

            try
            {
                BSPDClose(bspdctx);
            }
            catch
            {
            }

            int i = 0;
            while (i < 1000)
            {
                BSPDClose(bspdctx);
                System.Threading.Thread.Sleep(3);
                i++;
            }

        }
    }
}
