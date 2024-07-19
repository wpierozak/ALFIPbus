## Input message format (from FRED)
```
reset\n<SWT_PAYLOAD>,write\nread
```

### Output message format (to FRED)

#### Success
```
success 0\n0x00000000000<PAYLOAD>
```
#### Failute
```
failure
```