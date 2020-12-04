## When working with Binance Futures API, REST requests sometimes are interrupted with socket error 
```
"12030 the connection with the server was terminated abnormally"
```

### This happens mostly in Tokyo VPS locations, much rare in Europe locations.
### The issue happens regularly from about 25.11.2020 

### This test case running on vultr Tokyo VPS produced the following output (at the time of this writing, 04:12.2020)

```
04:03:35: 
Error 12030 has occurred.   <fapi/v1/ticker/bookTicker>
04:23:55: 
Error 12030 has occurred.   <fapi/v1/ticker/bookTicker>
04:27:06: 
Error 12030 has occurred.   <fapi/v1/ticker/bookTicker>
04:49:07: 
Error 12030 has occurred.   <fapi/v1/depth?symbol=SOLUSDT&limit=500>
04:57:46: 
Error 12030 has occurred.   <fapi/v1/depth?symbol=SOLUSDT&limit=500>
05:03:23: 
Error 12030 has occurred.   <fapi/v1/time>
05:17:36: 
Error 12030 has occurred.   <fapi/v1/ticker/bookTicker>
05:29:27: 
Error 12030 has occurred.   <fapi/v1/klines?symbol=SOLUSDT&interval=5m>
```

