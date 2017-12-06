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
            public string   inputPath;
            public IntPtr   options;
            public int codedWidth;
            public int codedHeight;
            public int width;
            public int height;
            public int ysize;
            public int opcode;
            public int timeStamp;
            public int vFps;
            public int vDuration;
            public BSPDLogCallback logCallback;
        }


        [DllImport("BSPD")]
        private static extern IntPtr BSPDCreateCtx();

        [DllImport("BSPD")]
        private static extern int BSPDSetLogCallback(IntPtr ctx,BSPDLogCallback callback);

        [DllImport("BSPD")]
        private static extern int BSPDClose(IntPtr ctx);

        [DllImport("BSPD")]
        private static extern int BSPDOpen(IntPtr ctx, string input, string options);

        [DllImport("BSPD")]
        private static extern int BSPDGetYUV(IntPtr ctx, byte[] ydata, byte[] udata, byte[] vdata);

        [DllImport("BSPD")]
        private static extern int BSPDGetYUVWithTime(IntPtr ctx, byte[] ydata, byte[] udata, byte[] vdata,ref long vpts,ref long apts,ref long vduration ,ref long aduration);

        static void LogCallbackFunc(string log)
        {
            Console.Write(log);
        }

        static string yuvfilepath = "./test.yuv";
        //static string input = "http://127.0.0.1/vod/abc.flv";
        //static string input = "http://127.0.0.1/vod/test.ogv";
        static string input = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
        static void Main(string[] args)
        {

            IntPtr bspdctx = BSPDCreateCtx();

            BSPDSetLogCallback(bspdctx, LogCallbackFunc);
            BSPDOpen(bspdctx, input, null);

            var ctx = (BSPDContext)Marshal.PtrToStructure(bspdctx, typeof(BSPDContext));

            byte[] ydata = new byte[ctx.ysize];
            byte[] udata = new byte[ctx.ysize/4];
            byte[] vdata = new byte[ctx.ysize/4];

            long vpst =0, vdurations =0;

            var yuvfile =  System.IO.File.Open(yuvfilepath, System.IO.FileMode.OpenOrCreate);

            BSPDGetYUVWithTime(bspdctx, ydata, udata, vdata, ref vpst, ref vpst, ref vdurations, ref vdurations);
            int i = 0;
            while (BSPDGetYUV(bspdctx,ydata,udata,vdata)==0&&i<100)
            {
                //if (yuvfile!= null)
                //{
                //    yuvfile.Write(ydata, 0, ydata.Length);
                //    yuvfile.Write(udata, 0, udata.Length);
                //    yuvfile.Write(vdata, 0, vdata.Length);
                //}
                i++;
            }
           BSPDClose(bspdctx);

            yuvfile.Flush();
            yuvfile.Close();


        }
    }
}
