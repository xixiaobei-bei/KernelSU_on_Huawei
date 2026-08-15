//****************************************************************************** 
// Copyright     :  Copyright (C) 2017, Hisilicon Technologies Co., Ltd.
// File name     :  cmdlst_reg_offset.h
// Author        :  AnthonySixta
// Version       :  1.0 
// Date          :  2017-06-17
// Description   :  Define all registers/tables for HiStarISP
// Others        :  Generated automatically by nManager V4.0 
// History       :  AnthonySixta 2017-06-17 Create file
//******************************************************************************

#ifndef __CMDLST_REG_OFFSET_H__
#define __CMDLST_REG_OFFSET_H__


/******************************************************************************/
/*                      HiStarISP CMDLST Registers' Definitions                            */
/******************************************************************************/

#define CMDLST_ID_REG                     0x0   /* Module Identifier */                                                                      
#define CMDLST_VERSION_REG                0x4   /* Module Version */                                                                         
#define CMDLST_CFG_REG                    0x20  /* Module configuration */                                                                   
#define CMDLST_DEBUG_0_REG                0x40  /* Debug register (available over the debug bus, lower part) */                              
#define CMDLST_DEBUG_1_REG                0x44  /* Debug register */                                                                         
#define CMDLST_DEBUG_2_REG                0x48  /* Debug register (available over the debug bus, upper part) */                              
#define CMDLST_DEBUG_3_REG                0x4C  /* Debug register */                                                                         
#define CMDLST_DEBUG_4_REG                0x50  /* Debug register */                                                                         
#define CMDLST_VCD_TRACE_REG              0x54  /* Configure VCD trace signals to output */  
#define CMDLST_CH_CFG_0_REG               0x80  /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_1_REG               0x100 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_2_REG               0x180 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_3_REG               0x200 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_4_REG               0x280 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_5_REG               0x300 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_6_REG               0x380 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_7_REG               0x400 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_8_REG               0x480 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_9_REG               0x500 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_10_REG              0x580 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_11_REG              0x600 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_12_REG              0x680 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_13_REG              0x700 /* Channel configuration register. */                                                        
#define CMDLST_CH_CFG_14_REG              0x780 /* Channel configuration register. */                                                        
#define CMDLST_SW_CH_MNGR_0_REG           0x84  /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_1_REG           0x104 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_2_REG           0x184 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_3_REG           0x204 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_4_REG           0x284 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_5_REG           0x304 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_6_REG           0x384 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_7_REG           0x404 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_8_REG           0x484 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_9_REG           0x504 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_10_REG          0x584 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_11_REG          0x604 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_12_REG          0x684 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_13_REG          0x704 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CH_MNGR_14_REG          0x784 /* Allows to control the priority of the channel by SW */                                    
#define CMDLST_SW_CVDR_RD_ADDR_0_REG      0x88  /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_1_REG      0x108 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_2_REG      0x188 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_3_REG      0x208 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_4_REG      0x288 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_5_REG      0x308 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_6_REG      0x388 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_7_REG      0x408 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_8_REG      0x488 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_9_REG      0x508 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_10_REG     0x588 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_11_REG     0x608 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_12_REG     0x688 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_13_REG     0x708 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_ADDR_14_REG     0x788 /* CVDR Video port read address to write the configuration data (CMDLST_SW_CVDR_RD_DATA*) */ 
#define CMDLST_SW_CVDR_RD_DATA_0_0_REG    0x8C  /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_1_REG    0x10C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_2_REG    0x18C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_3_REG    0x20C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_4_REG    0x28C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_5_REG    0x30C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_6_REG    0x38C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_7_REG    0x40C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_8_REG    0x48C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_9_REG    0x50C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_10_REG   0x58C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_11_REG   0x60C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_12_REG   0x68C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_13_REG   0x70C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_0_14_REG   0x78C /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_0_REG    0x90  /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_1_REG    0x110 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_2_REG    0x190 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_3_REG    0x210 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_4_REG    0x290 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_5_REG    0x310 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_6_REG    0x390 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_7_REG    0x410 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_8_REG    0x490 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_9_REG    0x510 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_10_REG   0x590 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_11_REG   0x610 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_12_REG   0x690 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_13_REG   0x710 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_1_14_REG   0x790 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_0_REG    0x94  /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_1_REG    0x114 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_2_REG    0x194 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_3_REG    0x214 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_4_REG    0x294 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_5_REG    0x314 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_6_REG    0x394 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_7_REG    0x414 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_8_REG    0x494 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_9_REG    0x514 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_10_REG   0x594 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_11_REG   0x614 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_12_REG   0x694 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_13_REG   0x714 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_2_14_REG   0x794 /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_SW_CVDR_RD_DATA_3_0_REG    0x98  /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_1_REG    0x118 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_2_REG    0x198 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_3_REG    0x218 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_4_REG    0x298 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_5_REG    0x318 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_6_REG    0x398 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_7_REG    0x418 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_8_REG    0x498 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_9_REG    0x518 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_10_REG   0x598 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_11_REG   0x618 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_12_REG   0x698 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_13_REG   0x718 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_CVDR_RD_DATA_3_14_REG   0x798 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_SW_BRANCH_0_REG            0xA0  /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_1_REG            0x120 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_2_REG            0x1A0 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_3_REG            0x220 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_4_REG            0x2A0 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_5_REG            0x320 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_6_REG            0x3A0 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_7_REG            0x420 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_8_REG            0x4A0 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_9_REG            0x520 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_10_REG           0x5A0 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_11_REG           0x620 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_12_REG           0x6A0 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_13_REG           0x720 /* Branching request */                                                                      
#define CMDLST_SW_BRANCH_14_REG           0x7A0 /* Branching request */                                                                      
#define CMDLST_LAST_EXEC_RD_DATA_3_0_REG  0xA4  /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_1_REG  0x124 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_2_REG  0x1A4 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_3_REG  0x224 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_4_REG  0x2A4 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_5_REG  0x324 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_6_REG  0x3A4 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_7_REG  0x424 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_8_REG  0x4A4 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_9_REG  0x524 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_10_REG 0x5A4 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_11_REG 0x624 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_12_REG 0x6A4 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_13_REG 0x724 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_LAST_EXEC_RD_DATA_3_14_REG 0x7A4 /* last CVDR_RD_DATA_3 information used to configure the CVDR */                             
#define CMDLST_HW_CH_MNGR_0_REG           0xBC  /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_1_REG           0x13C /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_2_REG           0x1BC /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_3_REG           0x23C/* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_4_REG           0x2BC /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_5_REG           0x33C /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_6_REG           0x3BC /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_7_REG           0x43C /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_8_REG           0x4BC /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_9_REG           0x53C /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_10_REG          0x5BC /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_11_REG          0x63C /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_12_REG          0x6BC /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_13_REG          0x73C /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CH_MNGR_14_REG          0x7BC /* Allows to control the priority of the channel by HW command list queues */                
#define CMDLST_HW_CVDR_RD_ADDR_0_REG      0xC0  /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_1_REG      0x140 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_2_REG      0x1C0 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_3_REG      0x240 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_4_REG      0x2C0 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_5_REG      0x340 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_6_REG      0x3C0 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_7_REG      0x440 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_8_REG      0x4C0 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_9_REG      0x540 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_10_REG     0x5C0 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_11_REG     0x640 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_12_REG     0x6C0 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_13_REG     0x740 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_ADDR_14_REG     0x7C0 /* CVDR Video port read address to write the configuration data (CMDLST_HW_CVDR_RD_DATA*) */ 
#define CMDLST_HW_CVDR_RD_DATA_0_0_REG    0xC4  /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_1_REG    0x144 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_2_REG    0x1C4 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_3_REG    0x244 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_4_REG    0x2C4 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_5_REG    0x344 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_6_REG    0x3C4 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_7_REG    0x444 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_8_REG    0x4C4 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_9_REG    0x544 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_10_REG   0x5C4 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_11_REG   0x644 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_12_REG   0x6C4 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_13_REG   0x744 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_0_14_REG   0x7C4 /* 1st Data to write (VP_RD_CFG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_0_REG    0xC8  /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_1_REG    0x148 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_2_REG    0x1C8 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_3_REG    0x248 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_4_REG    0x2C8 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_5_REG    0x348 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_6_REG    0x3C8 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_7_REG    0x448 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_8_REG    0x4C8 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_9_REG    0x548 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_10_REG   0x5C8 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_11_REG   0x648 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_12_REG   0x6C8 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_13_REG   0x748 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_1_14_REG   0x7C8 /* 2nd data to write (VP_RD_LWG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_0_REG    0xCC  /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_1_REG    0x14C /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_2_REG    0x1CC /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_3_REG    0x24C /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_4_REG    0x2CC /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_5_REG    0x34C /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_6_REG    0x3CC /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_7_REG    0x44C /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_8_REG    0x4CC /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_9_REG    0x54C /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_10_REG   0x5CC /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_11_REG   0x64C /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_12_REG   0x6CC /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_13_REG   0x74C /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_2_14_REG   0x7CC /* 3rd Data to write (VP_RD_FHG) */                                                          
#define CMDLST_HW_CVDR_RD_DATA_3_0_REG    0xD0  /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_1_REG    0x150 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_2_REG    0x1D0 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_3_REG    0x250 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_4_REG    0x2D0 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_5_REG    0x350 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_6_REG    0x3D0 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_7_REG    0x450 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_8_REG    0x4D0 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_9_REG    0x550 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_10_REG   0x5D0 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_11_REG   0x650 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_12_REG   0x6D0 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_13_REG   0x750 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_HW_CVDR_RD_DATA_3_14_REG   0x7D0 /* 4th Data to write (VP_RD_AXI_FS) */                                                       
#define CMDLST_CHANNEL_DEBUG_0_REG        0xE0  /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_1_REG        0x160 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_2_REG        0x1E0 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_3_REG        0x260 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_4_REG        0x2E0 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_5_REG        0x360 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_6_REG        0x3E0 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_7_REG        0x460 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_8_REG        0x4E0 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_9_REG        0x560 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_10_REG       0x5E0 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_11_REG       0x660 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_12_REG       0x6E0 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_13_REG       0x760 /* Debug register */                                                                         
#define CMDLST_CHANNEL_DEBUG_14_REG       0x7E0 /* Debug register */                                                                         

#endif // __CMDLST_REG_OFFSET_H__

