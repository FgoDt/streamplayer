using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BSPDTest
{
    class BaseClock
    {
        public long LastUpdate { get; set; }
        public long StartTime { get; set; }
    }

    class VideoClock:BaseClock
    {
        public long Pts { get; set; }
        public long Dur { get; set; }
        public long FirstPts { get; set; }
        public long RenderTime { get; set; }

        public void SetClock(long pts, long dur, long firstPts)
        {
            Pts = pts;
            Dur = dur;
            FirstPts = firstPts;
            StartTime = pts;
        }

        public void Update(long pst, long dur, long lastUpdate)
        {
            Pts = pst;
            Dur = dur;
            LastUpdate = lastUpdate;
        }

        private long getRenderTime()
        {
            long rt = Pts + Dur;
            return rt;
        }

    }
}
