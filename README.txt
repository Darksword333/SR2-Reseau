// Léger bug conçernant le dernier paquet, si non reception du dernier
// paquet par le récepteur, l'émetteur quoiqu'il arrive tente de le
// renvoyer n fois (max_try = 10) mais il peut arriver que n fois 
// n'est pas suffisant donc l'émetteur se termine et le recepteur attend 
// toujours le dernier paquet.