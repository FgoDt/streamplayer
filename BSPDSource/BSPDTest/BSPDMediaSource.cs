using System;
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

       public enum BSPDMediaState
        {
            open,
            buffering,
            close,
            error
        }

        public delegate void BSPDMediaStateChange(BSPDMediaState sate);

        public event BSPDMediaStateChange MediaStateChage;

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

        [DllImport("BSPD")]
        private static extern int BSPDGetPacket(IntPtr ctx, IntPtr pkt);

        [DllImport("BSPD")]
        private static extern IntPtr BSPDCreatePacket(IntPtr ctx, ref int op);

        [DllImport("BSPD")]
        private static extern int BSPDAbort();

        [DllImport("BSPD")]
        private static extern int BSPDGetRawDataWithTime(IntPtr ctx, byte[] ydata, byte[] udata, byte[] ydat, ref long pts, ref long duration);

        static void LogCallbackFunc(string log)
        {
            Console.Write(log);
        }

        int GetInt32(byte[] data)
        {
            if (data == null &&data.Length<4)
            {
                return -1;
            }
            int ret = 0;
            ret = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + (data[3]);
            return ret;
        }

        object mctxlocker = new object();

        //  BSPDStaticCache StaticCache;
        BSPDMediaCache mediaCache;
        BSPDCache bSPDCache;
        public string PATH;
        BSPDTimeAsyncControl tcontrol;
        public bool OpenMedia(string url)
        {
            mediaCache = new BSPDMediaCache();
            //StaticCache = new BSPDStaticCache();
            tcontrol = new BSPDTimeAsyncControl();
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
        int idex = -1;
        public void CheckUpdate()
        {
            curtime = GetTimeStamp();
           
            if (bSPDCache!=null&&nextData ==null)
            {
                //idex = StaticCache.PopIndex();
                //if (idex!=-1)
                //{
                //    nextData = StaticCache.mediaQueue[idex];
                //    StaticCache.Pop();
                //}
                //else
                //{
                //    nextData = null;
                //}
                if (tcontrol.DelayTime>500)
                {
                    mediaCache.TrySkipFrame(-1);
                }
                nextData = mediaCache.Pop();
//                nextData = bSPDCache.Pop(BSPDMediaDataType.Video);
            }
            if (preData == null&&nextData != null)
            {
                if (RBaseTime == 0)
                {
                    RBaseTime = curtime;
                }
                preData = nextData;
                pretime = curtime;
                MediaStateChage?.Invoke(BSPDMediaState.open);
                Update();
                tcontrol.SetInitTime(preData.vpts);
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
            tcontrol.CheckUpdate(preData);
            //Console.WriteLine(preData.vpts);
            //if (VBaseTime ==-1)
            //{
            //  vPaseTime =  VBaseTime = preData.vpts;
            //}
            //vPaseTime += preData.vdur;
            //Console.WriteLine("DUR:" + preData.vdur);
            nextData = null;
        }

        public void Close()
        {
            BSPDAbort();
            if (mThread!=null)
            {
                mThread.Abort();
            }
            lock (mctxlocker)
            {
                BSPDClose(mBSPDCtx);
            }
            MediaStateChage?.Invoke(BSPDMediaState.close);
        }

        IntPtr mBSPDCtx;
        private void StartThread()
        {

            mBSPDCtx = BSPDCreateCtx();
            GC.SuppressFinalize(mBSPDCtx);
            BSPDOpen(mBSPDCtx, PATH, "-d -timeout 1000");

            //int op = 123;
            //IntPtr pkt = BSPDCreatePacket(mBSPDCtx, ref op);
            //op = BSPDGetPacket(mBSPDCtx, pkt);

            //while (true)
            //{
            //    op = BSPDGetPacket(mBSPDCtx, pkt);
            //}

         //   BSPDSetLogCallback(mBSPDCtx, LogCallbackFunc);
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

            var  file = System.IO.File.Open("e:/test.pcm", System.IO.FileMode.OpenOrCreate);

            BSPDContext bSPD = (BSPDContext)Marshal.PtrToStructure(mBSPDCtx, typeof(BSPDContext));
            GC.SuppressFinalize(bSPD);
            ydata = new byte[bSPD.ysize];
            udata = new byte[bSPD.ysize / 4];
            vdata = new byte[bSPD.ysize / 4];
            long apts = 0, vpts = 0, aduration = 0, vduration = 0;
            MediaStateChage?.Invoke(BSPDMediaState.buffering);
            while (true)
            {

                //var idex= StaticCache.PushIndex();
                //if (idex == -1)
                //{
                //    Thread.Sleep(10);
                //    continue;
                //}
                
                //if (mediaData== null)
                //{
                //    StaticCache.mediaQueue[idex] = new BSPDMediaData();
                //    StaticCache.mediaQueue[idex].YData = new byte[ydata.Length];
                //    StaticCache.mediaQueue[idex].VData = new byte[vdata.Length];
                //    StaticCache.mediaQueue[idex].UData = new byte[udata.Length];
                //}


                if (op)
                {
                    int flt = 0;
                    lock (mctxlocker)
                    {
                        //if(0 != BSPDGetYUVWithTime(mBSPDCtx, ydata, udata, vdata, ref vpts, ref apts, ref vduration, ref aduration))
                        //{
                        //    MediaStateChage?.Invoke(BSPDMediaState.error);
                        //}
                        flt = BSPDGetRawDataWithTime(mBSPDCtx, ydata, udata, vdata, ref vpts, ref vduration);
                        if(flt == 2)
                        {
                            int num = GetInt32(udata);
                            file.Write(ydata, 0, num);
                            continue;
                        }
                        else if (flt == 1)
                        {
                        }
                        else
                        {
                            break;
                        }
                    }
                    if (flt == 1)
                    {
                        mediaData = new BSPDMediaData();
                        mediaData.YData = new byte[ydata.Length];
                        mediaData.VData = new byte[vdata.Length];
                        mediaData.UData = new byte[udata.Length];
                        mediaData.vpts = vpts;
                        mediaData.vdur = vduration;
                    }
                    //Array.Copy(ydata, StaticCache.mediaQueue[idex].YData, ydata.Length);
                    //Array.Copy(vdata, StaticCache.mediaQueue[idex].VData, vdata.Length);
                    //Array.Copy(udata, StaticCache.mediaQueue[idex].UData, udata.Length);
                    //StaticCache.mediaQueue[idex].vpts = vpts;
                    //StaticCache.mediaQueue[idex].vdur = vduration;
                    //StaticCache.mediaQueue[idex].MediaType = BSPDMediaDataType.Video;
                }
                else
                {
                    Thread.Sleep(10);
                }
                op = mediaCache.Push(mediaData);
               // op = StaticCache.Push();
                //op = bSPDCache.Push(mediaData);
            }
        }
        
    }

}
