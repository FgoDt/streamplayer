using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BSPDTest
{
    class BSPDTimeControl
    {

        VideoClock videoClock = new VideoClock();
        VideoClock audioClock = new VideoClock();
        RealTimeClock realTimeClock = new RealTimeClock();

        public long LagTime { get; set; }
        public long RenderAvgtime { get; set; }
        public long RenderNum { get; set; }


        private static long GetTimeStamp()
        {
            TimeSpan ts = DateTime.UtcNow - new DateTime(1970, 1, 1, 0, 0, 0, 0);
            return Convert.ToInt64(ts.TotalMilliseconds);
        }

        public void StartControl(long vfpts, long afpts)
        {
            videoClock.FirstPts = vfpts;
            audioClock.FirstPts = afpts;
        }

        public void CalculateAVGTime()
        {
            RenderNum++;
            RenderAvgtime = (realTimeClock.LastUpdate - realTimeClock.StartTime) / RenderNum;
        }

        public long GetMediaDur(BSPDMediaData data)
        {
            long st = 0;
            if (data.MediaType == BSPDMediaDataType.Audio)
            {
                st = audioClock.StartTime;
            }
            else 
            {
                st = videoClock.StartTime;
            }
            long rt = data.vpts - st + data.vdur;
            return rt;
        }

        public long GetRealTimeDurWithoutLag()
        {
            long rt = realTimeClock.LastUpdate - realTimeClock.StartTime + LagTime;
            return rt;
        }

        public void UpdateMediaClock(BSPDMediaData data,long time)
        {
            if (data.MediaType == BSPDMediaDataType.Audio)
            {
                audioClock.Update(data.vpts, data.vdur, time);
            }
            else
            {
                videoClock.Update(data.vpts, data.vdur, time);
            }
        }

        public void InitClock(BSPDMediaData adata, BSPDMediaData vdata)
        {
            long time = GetTimeStamp();
            realTimeClock.SetClock(time);
            realTimeClock.UpDateClock(time);
            videoClock.SetClock(vdata.vpts, vdata.vdur, vdata.vpts);
           // audioClock.SetClock(adata.vpts, adata.vdur, adata.vpts);
        }

        public int CheckUpdate(BSPDMediaData data)
        {
            long time = GetTimeStamp();
            
            long delay = time - realTimeClock.LastUpdate;
            long mediadur = GetMediaDur(data);
            long rtdur = GetRealTimeDurWithoutLag();
            long diff = mediadur - rtdur - delay;
            CalculateAVGTime();
            realTimeClock.UpDateClock(time);

            if (diff<-RenderAvgtime)
            {
                Console.WriteLine("drop data:" + data.MediaType +" diff:" +diff);
                int rt = 0;
                if ( -diff > RenderAvgtime + data.vdur)
                {
                    rt = (int)( -diff) / (int)data.vdur;
                }
                else
                {
                    rt = (int)( RenderAvgtime) / (int)data.vdur;
                }
                rt++;
                Console.WriteLine("drop data:" + data.MediaType +" rt:" + rt);
                return -rt;
            }
            diff = mediadur - rtdur - delay - RenderAvgtime;
            if (diff>0)
            {
                Console.WriteLine("NotNeed Update diff :" + diff);
                return 0;
            }
            else
            {
                UpdateMediaClock(data, time);
                Console.WriteLine("update");
                return 1;
            }

        }
    }
}