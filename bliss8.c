/*
ANSI C code generated by SmallEiffel The GNU Eiffel Compiler
Release -0.75 (July 16th 2001)
Copyright (C), 1994-2001 - LORIA - UHP - INRIA - FRANCE
Dominique COLNET and Suzanne COLLIN - SmallEiffel@loria.fr
http://SmallEiffel.loria.fr
*/
#ifdef __cplusplus
}
#endif
#include "bliss.h"
#ifdef __cplusplus
extern "C" {
#endif


void X35init(T0*C){
{Tid id=((T0*)C)->id;
if(id<=72){
if(id<=69){
if(id<=67){
/*[IRF3.1init*//*]*/
}
else{
r69init(((T69*)C));
}}
else{
if(id<=71){
r71init(((T71*)C));
}
else{
/*[IRF3.1init*//*]*/
}}}
else{
if(id<=74){
if(id<=73){
/*[IRF3.1init*//*]*/
}
else{
r74init(((T74*)C));
}}
else{
r75init(((T75*)C));
}}}
}


void X35init_connection_lists(T0*C){
{Tid id=((T0*)C)->id;
if(id<=72){
if(id<=69){
if(id<=67){
r67init_connection_lists(((T67*)C));
}
else{
r69init_connection_lists(((T69*)C));
}}
else{
if(id<=71){
r71init_connection_lists(((T71*)C));
}
else{
r72init_connection_lists(((T72*)C));
}}}
else{
if(id<=74){
if(id<=73){
r73init_connection_lists(((T73*)C));
}
else{
r74init_connection_lists(((T74*)C));
}}
else{
r75init_connection_lists(((T75*)C));
}}}
}


void X35init_pattern_table(T0*C){
{Tid id=((T0*)C)->id;
if(id<=72){
if(id<=69){
if(id<=67){
r67init_pattern_table(((T67*)C));
}
else{
r69init_pattern_table(((T69*)C));
}}
else{
if(id<=71){
r71init_pattern_table(((T71*)C));
}
else{
r72init_pattern_table(((T72*)C));
}}}
else{
if(id<=74){
if(id<=73){
r73init_pattern_table(((T73*)C));
}
else{
r74init_pattern_table(((T74*)C));
}}
else{
r75init_pattern_table(((T75*)C));
}}}
}


T6 X35is_sink(T0*C){
T6 R;
{Tid id=((T0*)C)->id;
if(id<=72){
if(id<=69){
if(id<=67){
R=(0);
}
else{
R=(1);
}}
else{
if(id<=71){
R=(0);
}
else{
R=(1);
}}}
else{
if(id<=74){
if(id<=73){
R=(1);
}
else{
R=(0);
}}
else{
R=(0);
}}}
return R;
}


void X35compute_next_frame(T0*C){
{Tid id=((T0*)C)->id;
if(id<=72){
if(id<=69){
if(id<=67){
r67compute_next_frame(((T67*)C));
}
else{
r69compute_next_frame(((T69*)C));
}}
else{
if(id<=71){
r71compute_next_frame(((T71*)C));
}
else{
r72compute_next_frame(((T72*)C));
}}}
else{
if(id<=74){
if(id<=73){
r73compute_next_frame(((T73*)C));
}
else{
r74compute_next_frame(((T74*)C));
}}
else{
r75compute_next_frame(((T75*)C));
}}}
}


void X35put_name(T0*C,T0* a1){
{Tid id=((T0*)C)->id;
if(id<=72){
if(id<=69){
if(id<=67){
r67put_name(((T67*)C),a1);
}
else{
r69put_name(((T69*)C),a1);
}}
else{
if(id<=71){
r71put_name(((T71*)C),a1);
}
else{
r72put_name(((T72*)C),a1);
}}}
else{
if(id<=74){
if(id<=73){
r73put_name(((T73*)C),a1);
}
else{
r74put_name(((T74*)C),a1);
}}
else{
r75put_name(((T75*)C),a1);
}}}
}


void X35init_picture(T0*C){
{Tid id=((T0*)C)->id;
if(id<=72){
if(id<=69){
if(id<=67){
r67init_picture(((T67*)C));
}
else{
r69init_picture(((T69*)C));
}}
else{
if(id<=71){
r71init_picture(((T71*)C));
}
else{
r72init_picture(((T72*)C));
}}}
else{
if(id<=74){
if(id<=73){
r73init_picture(((T73*)C));
}
else{
r74init_picture(((T74*)C));
}}
else{
r75init_picture(((T75*)C));
}}}
}


T0* T2toT27(T2 source){
T27*destination=new27();
destination->_item=source;
return ((T0*)destination);
}

#ifdef __cplusplus
}
#endif
