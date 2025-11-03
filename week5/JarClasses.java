/* ************************************************************************
> File Name:     JarClasses.java
> Author:        Zhenyu Song
> Mail:          zhenyus4@uci.edu
> Created Time:  Sun Nov  2 17:59:16 2025
> Description:   
 ************************************************************************/

import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class JarClasses {
    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("wrong usage, please enter: java JarClasses <jarfilename>");
            System.exit(1);
        }
        String jarPath = args[0];

        List<String> classNames = new ArrayList<>();

        JarFile jarFile = new JarFile(jarPath);
        Enumeration<JarEntry> entries = jarFile.entries();
        while (entries.hasMoreElements()) {
            JarEntry entry = entries.nextElement();
            if (!entry.isDirectory() && entry.getName().endsWith(".class")) {
                String className = entry.getName()
                        .replace(".class", "")
                        .replace('/', '.');
                classNames.add(className);
            }
        }
        jarFile.close();

        Collections.sort(classNames);

        URL jarUrl = Paths.get(jarPath).toUri().toURL();
        URL[] urls = { jarUrl };

        URLClassLoader classLoader = new URLClassLoader(urls);

        for (String className : classNames) {
            try {
                Class<?> loadedClass = classLoader.loadClass(className);

                int publicMethods = 0;
                int privateMethods = 0;
                int protectedMethods = 0;
                int staticMethods = 0;

                Method[] methods = loadedClass.getDeclaredMethods();
                for (Method method : methods) {
                    int modifiers = method.getModifiers();
                    if (Modifier.isPublic(modifiers)) {
                        publicMethods++;
                    }
                    if (Modifier.isPrivate(modifiers)) {
                        privateMethods++;
                    }
                    if (Modifier.isProtected(modifiers)) {
                        protectedMethods++;
                    }
                    if (Modifier.isStatic(modifiers)) {
                        staticMethods++;
                    }
                }

                Field[] fields = loadedClass.getDeclaredFields();
                int fieldCount = fields.length;

                System.out.println("----------" + className + "----------");
                System.out.println("  Public methods: " + publicMethods);
                System.out.println("  Private methods: " + privateMethods);
                System.out.println("  Protected methods: " + protectedMethods);
                System.out.println("  Static methods: " + staticMethods);
                System.out.println("  Fields: " + fieldCount);
            } catch (ClassNotFoundException | NoClassDefFoundError | UnsatisfiedLinkError e) {
                // silently ignore classes that cannot be loaded
            } catch (Exception e) {
                System.err.println("class: " + className + ", error: " + e.getMessage());
            }
        }

        classLoader.close();
    }
}