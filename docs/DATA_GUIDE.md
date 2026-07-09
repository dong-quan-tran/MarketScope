# LOBSTER Data Guide

## Source
https://data.lobsterdata.com/info/DataSamples.php

## Message file columns
1. Time
2. Type
3. Order ID
4. Size
5. Price
6. Direction

## Event types
- 1 = Add
- 2 = Partial cancel
- 3 = Cancel
- 4 = Execute
- 5 = Hidden execute

## Notes
- Prices are usually stored as integer * 10000
- Divide by 10000 to recover actual dollar prices
- Use the message file to replay events
- Use the orderbook file to validate reconstruction
