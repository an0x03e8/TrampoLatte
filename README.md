# TrampoLatté

TrampoLatté is an AMSI & ETW bypass poc which utilizes trampolines to hook specific function and alter their execution behavior. 

## Internals

```
      AmsiScanBuffer        EtwpEventWriteFull
          │                       │
  [13-byte jmp r10]      [13-byte jmp r10]
          │                       │
   ┌──────┴──────┐          ┌─────┴─────┐
   │   ProxyAmsi │          │  RetStub  │
   │  (saves ctx)│          │   (C3)    │
   └──────┬──────┘          └─────┬─────┘
          ▼                       ▼
     returns              returns to caller
   AmsiScanBuffer 
        =  
  AMSI_RESULT_CLEAN

```

![image](https://github.com/user-attachments/assets/d77e27f9-ca9e-4767-9e63-5ec666ce46b8)
