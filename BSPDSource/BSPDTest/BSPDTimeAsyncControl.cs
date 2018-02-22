using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BSPDTest
{
    enum BSPDSyncMode
    {
        SkipMode,
        DropMode,
        Normal
    }

    class BSPDTimeAsyncControl
    {
        private long videoInitTime;
        private long videoCurTime;
        private long videoPreTime;
        private long realInitTime;
        private long realCurTime;
        private long realPreTime;
        public long DelayTime { get; set; }

        private static long GetTimeStamp()
        {
            TimeSpan ts = DateTime.UtcNow - new DateTime(1970, 1, 1, 0, 0, 0, 0);
            return Convert.ToInt64(ts.TotalMilliseconds);
        }

        public void SetInitTime(long vt)
        {
            videoInitTime = vt;
            realInitTime = GetTimeStamp();
        }

        public void CheckUpdate(BSPDMediaData data)
        {
            videoCurTime = data.vpts;
            realCurTime = GetTimeStamp();

            if (videoCurTime - videoPreTime < realCurTime - realPreTime)
            {
                //update
                ResetPreTime();
                Console.WriteLine("UPDATE");
            }

            DelayTime = (realCurTime - realInitTime) - (videoCurTime - videoInitTime);
            Console.WriteLine("DELAY:" + DelayTime);
            
        }

        private void ResetPreTime()
        {
            videoPreTime = videoCurTime;
            realPreTime = realCurTime;
        }

        

    }
}
