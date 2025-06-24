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
## Detection

- Started a custom ETW Consumer (right terminal) monitoring the .NET runtime

![image](https://github.com/user-attachments/assets/45bbcc57-a264-450c-b166-41a21b89c4e0)

- Executing assemblies through Havoc doesn't trigger any detection (ETW)

![image](https://github.com/user-attachments/assets/80a6020e-aa22-48bc-ad99-dc9accbb1089)

# Disclaimer

This is just a POC and is not OPSEC safe. 
