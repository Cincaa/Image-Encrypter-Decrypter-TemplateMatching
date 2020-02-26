# Image Encrypter/Decrypter & TemplateMatching
Documentation can be found in `Explicarea Proiectului.pdf`, but it's in Romanian so I will point out the essentials here.

## Encrypting/Decrypting
This part consists of reading a BMP file from the computer which will be encrypted, using a secret given key. The algorithm permutes the pixels (XORSHIFT32), therefore encrypt them accordingly to the secret key. Decryption part is basically the reverse.

## Recognise handwritten digits
This algorithm identifies similarities between different parts of the image and our given samples, using template matching technique.  Image processing in the early phases is similar to the first part.
