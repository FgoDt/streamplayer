﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Runtime.InteropServices;

namespace BSPDTest
{
    class BSPDMediaSource
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
            public IntPtr pCoder;
            public string inputPath;
            public IntPtr options;
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
        private static extern int BSPDSetLogCallback(IntPtr ctx, BSPDLogCallback callback);

        [DllImport("BSPD")]
        private static extern int BSPDClose(IntPtr ctx);

        [DllImport("BSPD")]
        private static extern int BSPDOpen(IntPtr ctx, string input, string options);

        [DllImport("BSPD")]
        private static extern int BSPDGetYUV(IntPtr ctx, byte[] ydata, byte[] udata, byte[] vdata);

        [DllImport("BSPD")]
        private static extern int BSPDGetYUVWithTime(IntPtr ctx, byte[] ydata, byte[] udata, byte[] vdata, ref long vpts, ref long apts, ref long vduration, ref long aduration);

        static void LogCallbackFunc(string log)
        {
            Console.Write(log);
        }


        object mctxlocker = new object();

        BSPDCache bSPDCache;
        public string PATH;
        public bool OpenMedia(string url)
        {
            PATH = url;
            StartThread();
            // throw new NotImplementedException();
            return false;
        }

        private static long GetTimeStamp()
        {
            TimeSpan ts = DateTime.UtcNow - new DateTime(1970, 1, 1, 0, 0, 0, 0);
            return Convert.ToInt64(ts.TotalMilliseconds);
        }

        BSPDMediaData preData;
        BSPDMediaData nextData;
        long pretime;
        long RBaseTime;
        long VBaseTime = -1;
        long vPaseTime = 0;
        long curtime;
        public void CheckUpdate()
        {
            curtime = GetTimeStamp();
           
            if (bSPDCache!=null&&nextData ==null)
            {
                nextData = bSPDCache.Pop(BSPDMediaDataType.Video);
            }
            if (preData == null&&nextData != null)
            {
                if (RBaseTime == 0)
                {
                    RBaseTime = curtime;
                }
                preData = nextData;
                pretime = curtime;
                Update();
            }
            else
            {
                if (nextData != null)
                {
                    var vt = nextData.vpts - preData.vpts;
                    var rt = curtime - pretime;
                   // Console.WriteLine((curtime + vPaseTime - RBaseTime));
                    vt = nextData.vpts-VBaseTime;
                    rt = curtime - RBaseTime;
                    if (vt<rt)
                    {
                        Console.WriteLine("VT:" + nextData.vpts);
                         Console.WriteLine("RT:" + rt);
                        preData = nextData;
                        pretime = curtime;
                        Update();
                    }
                }
            }
        }

        public void Update()
        {
            Console.WriteLine(preData.vpts);
            if (VBaseTime ==-1)
            {
              vPaseTime =  VBaseTime = preData.vpts;
            }
            vPaseTime += preData.vdur;
            Console.WriteLine("DUR:" + preData.vdur);
            nextData = null;
        }

        public void Close()
        {
            if (mThread!=null)
            {
                mThread.Abort();
            }
            lock (mctxlocker)
            {
                BSPDClose(mBSPDCtx);
            }
        }

        IntPtr mBSPDCtx;
        private void StartThread()
        {

            mBSPDCtx = BSPDCreateCtx();
            GC.SuppressFinalize(mBSPDCtx);
         //   BSPDSetLogCallback(mBSPDCtx, LogCallbackFunc);
            BSPDOpen(mBSPDCtx, PATH, "");
            bSPDCache = new BSPDCache();
            mThread = new Thread(new ThreadStart(DataThread));
            mThread.IsBackground = true;
            mThread.Start();
        }

        Thread mThread;
        private void DataThread()
        {
            bool op = true;
            BSPDMediaData mediaData = null;
            byte[] ydata, udata, vdata;

            BSPDContext bSPD = (BSPDContext)Marshal.PtrToStructure(mBSPDCtx, typeof(BSPDContext));
            GC.SuppressFinalize(bSPD);
            ydata = new byte[bSPD.ysize];
            udata = new byte[bSPD.ysize / 4];
            vdata = new byte[bSPD.ysize / 4];
            long apts = 0, vpts = 0, aduration = 0, vduration = 0;

            while (true)
            {

                if (op)
                {
                    lock (mctxlocker)
                    {
                        BSPDGetYUVWithTime(mBSPDCtx, ydata, udata, vdata, ref vpts, ref apts, ref vduration, ref aduration);
                    }
                    mediaData = new BSPDMediaData();
                    mediaData.YData = new byte[ydata.Length];
                    mediaData.VData = new byte[vdata.Length];
                    mediaData.UData = new byte[udata.Length];
                    Array.Copy(ydata, mediaData.YData, ydata.Length);
                    Array.Copy(vdata, mediaData.VData, vdata.Length);
                    Array.Copy(udata, mediaData.UData, udata.Length);
                    mediaData.vpts = vpts;
                    mediaData.vdur = vduration;
                    mediaData.MediaType = BSPDMediaDataType.Video;
                }
                else
                {
                    Thread.Sleep(10);
                }
                op = bSPDCache.Push(mediaData);
            }
        }
        
    }

}