using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BSPDTest
{
    class RealTimeClock:BaseClock
    {

        public void SetClock(long startTime)
        {
            StartTime = startTime;
        }

        public void UpDateClock(long lastUpdate)
        {
            LastUpdate = lastUpdate;
        }
    }
}
