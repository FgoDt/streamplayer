using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using System.Runtime.InteropServices;

namespace BSPDTest
{
    class Program
    {

        [DllImport("BSPD")]
        private static extern void BSPDTest();



        static string yuvfilepath = "./test.yuv";
        static string input = "http://live-yf-hdl.huomaotv.cn/live/5fILSX.flv?from=huomaoroom";
       // static string input = "http://182.150.11.187/live-tx-hdl.huomaotv.cn/live/9eiHIY23992.flv?from=huomaoroom&dispatch_from=ztc10.230.33.209&utime=1518487699647";
        static BSPDMediaSource BSPDMS;
        
        static void Main(string[] args)
        {
            //BSPDMS = new BSPDMediaSource();
            //BSPDMS.MediaStateChage += BSPDMS_MediaStateChage;
            //BSPDMS.OpenMedia(input);
          
            BSPDMS = new BSPDMediaSource();
            BSPDMS.OpenMedia(input);
            System.Threading.Thread.Sleep(10);
              

           // BSPDMS.Close();
            while (true)
            {
                BSPDMS.CheckUpdate();
                Thread.Sleep(33);
            //    BSPDMS.CheckUpdate();
            }

        }

        private static void BSPDMS_MediaStateChage(BSPDMediaSource.BSPDMediaState sate)
        {
            Console.WriteLine("state:" + sate);
        }
    }
}
