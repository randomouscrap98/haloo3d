for i in range(0, 64):
    print("case {}:".format(i))
    print("#undef _HTF")
    print("#define _HTF {}".format(i))
    print("#include \"haloo3d_trimacro.c\"")
    print("break;")
