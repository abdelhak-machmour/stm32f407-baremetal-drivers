# Fundamentals

Short experiments on the basics of the STM32F407 and of embedded C, done before writing the drivers. Each folder is an independent STM32CubeIDE project.

| Project | Topic | What to observe |
|---|---|---|
| [`001HelloWorld`](001HelloWorld/) | `printf` over SWO / ITM | The message in the SWV ITM Data Console |
| [`002SampleApp`](002SampleApp/) | Debugging a C program on target: bubble sort and insertion sort | Breakpoints, step-by-step execution, watching arrays in memory |
| [`005HSI_Measurement`](005HSI_Measurement/) | Clock tree: routing the HSI (16 MHz) to the **MCO1** pin (PA8), divided by 4, by writing RCC and GPIO registers directly | A 4 MHz square wave on PA8 with an oscilloscope or logic analyser |
| [`button_interrupt`](button_interrupt/) | EXTI interrupt on the user button (PA0) configured **with raw register pointers only**: RCC, SYSCFG, EXTI, NVIC; flag shared between ISR and `main()` | Button press counter in the ITM console |
| [`volatile_experiment`](volatile_experiment/) | Why `volatile` matters: polling an SRAM location without `volatile`, compiled at `-O3` | The loop is optimised away and never sees the memory change; adding `volatile` fixes it. Compare the disassembly (`.list` file) |

## Build

Import the folder into STM32CubeIDE (`File > Import > Existing Projects into Workspace`), build, then flash with the ST-LINK.
